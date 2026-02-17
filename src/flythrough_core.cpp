#include "flythrough_core.h"
#include <QDebug>
#include <QtMath>
#include <qgscoordinatetransform.h>
#include <qgsdistancearea.h>
#include <qgsproject.h>
#include <qgsrasteridentifyresult.h>
#include <qgsrasterlayer.h>
#include <qgsvectorlayer.h>

FlyThroughCore::FlyThroughCore(QObject *parent) : QObject(parent) {}

bool FlyThroughCore::generateKeyframes(QgsVectorLayer *pathLayer,
                                       QgsRasterLayer *demLayer,
                                       double speedKmh, double altitudeOffset,
                                       double pitch, bool useBanking) {
  mKeyframes.clear();

  if (!pathLayer || !demLayer)
    return false;

  // 1. Extract and Densify Path
  QgsGeometry pathGeom;
  // Iterate features (take first for now)
  QgsFeatureIterator it = pathLayer->getFeatures();
  QgsFeature fet;
  if (it.nextFeature(fet)) {
    pathGeom = fet.geometry();
  }

  if (pathGeom.isEmpty())
    return false;

  // Densify to ensure smooth follow
  QgsGeometry denseGeom = densifyPath(pathGeom, 10.0); // 10 meters interval

  // 2. Traverse coordinates
  auto vertices = denseGeom.vertices();
  if (vertices.hasNext()) {
    QgsPointXY prevPt;
    bool first = true;
    double currentTime = 0.0;
    double speedMs = speedKmh * 1000.0 / 3600.0;
    if (speedMs <= 0)
      speedMs = 10.0;
    QgsDistanceArea da;
    da.setSourceCrs(pathLayer->crs(),
                    QgsProject::instance()->transformContext());
    da.setEllipsoid(QgsProject::instance()->ellipsoid());

    // We need previous vertex to calc distance
    QgsPoint vertexPos;
    QgsPoint prevVertexPos;

    while (vertices.hasNext()) {
      vertexPos = vertices.next();
      QgsPointXY pt(vertexPos.x(), vertexPos.y());

      if (first) {
        prevPt = pt;
        prevVertexPos = vertexPos;
        first = false;
      } else {
        double dist = da.measureLine(prevPt, pt);
        currentTime += dist / speedMs;
      }

      // 3. Sample Elevation
      double terraZ = getElevation(demLayer, pt);
      double camZ = terraZ + altitudeOffset;

      // 4. Calculate Yaw (Bearing)
      double yaw = 0;
      if (currentTime > 0) {
        yaw = calculateBearing(prevPt, pt);
      }

      CameraKeyframe kf;
      kf.time = currentTime;
      kf.position =
          QgsVector3D(pt.x(), pt.y(),
                      camZ); // Note: 3D scene usually X, Z(up), -Y? OR X, Y, Z?
      // QGIS 3D API uses QgsVector3D(x, y, z) where Z is up in map coordinates.

      kf.yaw = yaw;
      kf.pitch = pitch;
      kf.roll = 0; // TODO: Implement banking logic

      mKeyframes.push_back(kf);

      prevPt = pt;
    }
  }

  if (!mKeyframes.empty()) {
    mTotalDuration = mKeyframes.back().time;
  }

  return !mKeyframes.empty();
}

QgsGeometry FlyThroughCore::densifyPath(const QgsGeometry &geom,
                                        double interval) {
  // Simple densification wrapper
  return geom.densifyByDistance(interval);
}

double FlyThroughCore::getElevation(QgsRasterLayer *dem,
                                    const QgsPointXY &point) {
  // Transform point if needed? Assuming project CRS context handling handled
  // outside for now Ideally should verify CRS matches.

  QgsRasterDataProvider *provider = dem->dataProvider();
  if (!provider)
    return 0.0;

  QgsPointXY samplePt = point;
  // Simple identify
  QgsRasterIdentifyResult result =
      provider->identify(samplePt, Qgis::RasterIdentifyFormat::Value);
  if (result.isValid()) {
    QMap<int, QVariant> results = result.results();
    if (results.contains(1)) { // Band 1
      bool ok;
      double val = results[1].toDouble(&ok);
      if (ok)
        return val;
    }
  }
  return 0.0;
}

double FlyThroughCore::calculateBearing(const QgsPointXY &p1,
                                        const QgsPointXY &p2) const {
  double dx = p2.x() - p1.x();
  double dy = p2.y() - p1.y();
  double bearing = qRadiansToDegrees(qAtan2(dx, dy));
  // Normalize to 0-360
  if (bearing < 0)
    bearing += 360.0;
  return bearing;
}

CameraKeyframe FlyThroughCore::interpolate(double time) const {
  CameraKeyframe result;
  if (mKeyframes.empty())
    return result;

  if (time <= 0)
    return mKeyframes.front();
  if (time >= mTotalDuration)
    return mKeyframes.back();

  // Find segment
  // Linear scan for now (optimize with binary search later)
  size_t idx = 0;
  while (idx < mKeyframes.size() - 1 && mKeyframes[idx + 1].time < time) {
    idx++;
  }

  const CameraKeyframe &k1 = mKeyframes[idx];
  const CameraKeyframe &k2 = mKeyframes[idx + 1];

  double t = (time - k1.time) / (k2.time - k1.time);

  // Linear Interpolation
  result.time = time;
  result.position =
      QgsVector3D(k1.position.x() + (k2.position.x() - k1.position.x()) * t,
                  k1.position.y() + (k2.position.y() - k1.position.y()) * t,
                  k1.position.z() + (k2.position.z() - k1.position.z()) * t);

  result.pitch = k1.pitch + (k2.pitch - k1.pitch) * t;
  result.yaw = lerpAngle(k1.yaw, k2.yaw, t);
  result.roll = k1.roll + (k2.roll - k1.roll) * t;

  return result;
}

double FlyThroughCore::lerpAngle(double a, double b, double t) const {
  double diff = b - a;
  // Shortest path interpolation
  while (diff > 180)
    diff -= 360;
  while (diff < -180)
    diff += 360;
  return a + diff * t;
}
