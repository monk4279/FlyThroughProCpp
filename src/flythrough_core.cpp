#include "flythrough_core.h"
#include <QApplication>
#include <QDebug>
#include <QMessageBox>
#include <QThread>
#include <QTimer>
#include <QtMath>
#include <cmath>
#include <qgisinterface.h>
// NOTE: Do NOT include qgs3dmapcanvas.h or qgscameracontroller.h here.
// Those headers would cause linkage against symbols not in QGIS 3.28.3.
// We use QMetaObject::invokeMethod for dynamic dispatch instead.
#include <qgs3dmapsettings.h>
#include <qgsapplication.h>
#include <qgscoordinatereferencesystem.h>
#include <qgscoordinatetransform.h>
#include <qgsdemterraingenerator.h>
#include <qgsdistancearea.h>
#include <qgsfeature.h>
#include <qgsfeatureiterator.h>
#include <qgsgeometry.h>
#include <qgslayertree.h>
#include <qgsmaplayer.h>
#include <qgsmessagebar.h>
#include <qgspointxy.h>
#include <qgsproject.h>
#include <qgsrasteridentifyresult.h>
#include <qgsrasterlayer.h>
#include <qgsvectorlayer.h>
#include <qgswkbtypes.h>

FlyThroughCore::FlyThroughCore(QgisInterface *iface, QObject *parent)
    : QObject(parent), mIface(iface) {}

FlyThroughCore::~FlyThroughCore() { close3DCanvas(); }

bool FlyThroughCore::generateFlythrough(const FlythroughParams &params) {
  try {
    // Validate inputs
    if (!params.demLayer || !params.pathLayer) {
      QMessageBox::warning(nullptr, "Missing Layers",
                           "Please select both DEM and path layers.");
      return false;
    }

    // Extract path vertices
    QList<QgsPointXY> vertices = extractPathVertices(params.pathLayer);
    if (vertices.size() < 2) {
      QMessageBox::warning(nullptr, "Invalid Path",
                           "Path must have at least 2 vertices.");
      return false;
    }

    qDebug() << "[FTP] Path has" << vertices.size() << "vertices";

    // Setup 3D canvas first (needed for CRS determination)
    if (!setup3DCanvas(params, vertices.first())) {
      return false;
    }

    // Transform vertices to View CRS
    QgsCoordinateReferenceSystem viewCRS = mMapSettings3D->crs();
    QgsCoordinateReferenceSystem pathCRS = params.pathLayer->crs();

    if (pathCRS != viewCRS) {
      qDebug() << "[FTP] Transforming path from" << pathCRS.authid() << "to"
               << viewCRS.authid();
      QgsCoordinateTransform xform(pathCRS, viewCRS, QgsProject::instance());
      for (int i = 0; i < vertices.size(); ++i) {
        vertices[i] = xform.transform(vertices[i]);
      }
    }

    // Generate keyframes
    generateKeyframes(vertices, params);

    if (mKeyframes.empty()) {
      QMessageBox::warning(nullptr, "Error", "Failed to generate keyframes.");
      return false;
    }

    // Setup animation
    setupAnimation(params);

    mIface->messageBar()->pushMessage("Flythrough Pro",
                                      "Animation started – watch the 3D view!",
                                      Qgis::MessageLevel::Info, 5);

    return true;

  } catch (const std::exception &e) {
    QMessageBox::critical(nullptr, "Error",
                          QString("An error occurred: %1").arg(e.what()));
    return false;
  }
}

void FlyThroughCore::stopAnimation() {
  if (mAnimTimer) {
    mAnimTimer->stop();
    mAnimTimer->deleteLater();
    mAnimTimer = nullptr;
  }
}

bool FlyThroughCore::setup3DCanvas(const FlythroughParams &params,
                                   const QgsPointXY &startPoint) {
  // Store params
  mCameraHeight = params.cameraHeight;
  mLookaheadDist = params.lookaheadDistance;
  mPitchAngle = params.cameraPitch;
  mVerticalScale = params.verticalExaggeration;
  mDemLayer = params.demLayer;

  // Close existing canvas
  close3DCanvas();

  // Try to createFrom interface (QGIS 3.30+) or find existing
  mCanvas3D = nullptr;

  if (mIface->metaObject()->indexOfMethod("createNewMapCanvas3D(QString)") >=
      0) {
    // Call dynamically - use QWidget* to avoid linking against Qgs3DMapCanvas
    QWidget *newCanvas = nullptr;
    QMetaObject::invokeMethod(
        mIface, "createNewMapCanvas3D", Qt::DirectConnection,
        Q_RETURN_ARG(QWidget *, newCanvas), Q_ARG(QString, "Flythrough Pro"));
    mCanvas3D = newCanvas;
  }

  if (!mCanvas3D) {
    qDebug() << "[FTP] Creating 3D canvas failed or unsupported. Searching for "
                "existing...";
    mCanvas3D = findExisting3DCanvas();
  }

  if (!mCanvas3D) {
    QMessageBox::warning(mIface->mainWindow(), "3D View Required",
                         "This version of QGIS does not support creating 3D "
                         "views programmatically.\n\n"
                         "Please open a 3D Map View manually:\n"
                         "View → 3D Map Views → New 3D Map View\n\n"
                         "Then try generating again.");
    return false;
  }

  // Let scene initialize
  for (int i = 0; i < 20; ++i) {
    QApplication::processEvents();
    QThread::msleep(50);
  }

  // Get settings via dynamic call (Qgs3DMapCanvas::mapSettings() may not be
  // exported in 3.28.3 - access via Qt's meta-object system or property)
  mMapSettings3D = nullptr;
  // Try to get mapSettings via dynamic invocation
  QMetaObject::invokeMethod(mCanvas3D, "mapSettings", Qt::DirectConnection,
                            Q_RETURN_ARG(Qgs3DMapSettings *, mMapSettings3D));

  if (!mMapSettings3D) {
    qDebug() << "[FTP] mapSettings() is nullptr, creating new...";
    mMapSettings3D = new Qgs3DMapSettings();
    // setMapSettings via dynamic call
    QMetaObject::invokeMethod(mCanvas3D, "setMapSettings", Qt::DirectConnection,
                              Q_ARG(Qgs3DMapSettings *, mMapSettings3D));
  }

  if (!mMapSettings3D) {
    QMessageBox::critical(nullptr, "Error",
                          "3D canvas has no map settings – please open a 3D "
                          "view manually first.");
    return false;
  }

  // Configure terrain using QGIS 3.28 API (QgsDemTerrainGenerator)
  if (params.demLayer) {
    QgsDemTerrainGenerator *terrainGen = new QgsDemTerrainGenerator();
    terrainGen->setLayer(params.demLayer);
    terrainGen->setCrs(mMapSettings3D->crs(),
                       QgsProject::instance()->transformContext());
    mMapSettings3D->setTerrainGenerator(terrainGen);
  }
  mMapSettings3D->setTerrainVerticalScale(params.verticalExaggeration);

  // CRS handling
  QgsCoordinateReferenceSystem projectCRS = QgsProject::instance()->crs();
  if (projectCRS.isGeographic()) {
    // Force EPSG:3857 for geographic projects
    QgsCoordinateReferenceSystem mercator("EPSG:3857");
    mMapSettings3D->setCrs(mercator);
    qDebug() << "[FTP] Project is Geographic, setting 3D View to EPSG:3857";
  } else {
    mMapSettings3D->setCrs(projectCRS);
  }

  // Set origin (transform start point if needed)
  QgsPointXY origin = startPoint;
  if (projectCRS != mMapSettings3D->crs() && projectCRS.isGeographic()) {
    QgsCoordinateTransform ct(projectCRS, mMapSettings3D->crs(),
                              QgsProject::instance());
    origin = ct.transform(startPoint);
  }

  mMapSettings3D->setOrigin(QgsVector3D(origin.x(), origin.y(), 0));

  // Layers (setExtent removed - not in QGIS 3.28.3 Qgs3DMapSettings API)
  QList<QgsMapLayer *> layers =
      QgsProject::instance()->layerTreeRoot()->layerOrder();
  if (params.overlayLayer && !layers.contains(params.overlayLayer)) {
    layers.append(params.overlayLayer);
  }
  mMapSettings3D->setLayers(layers);

  // Rendering options
  mMapSettings3D->setTerrainShadingEnabled(params.terrainShading);
  mMapSettings3D->setFieldOfView(params.fieldOfView);

  // Show canvas
  mCanvas3D->resize(1280, 720);
  mCanvas3D->show();

  qDebug() << "[FTP] 3D Canvas initialized. Origin:" << origin.toString();

  // Let terrain tiles load
  for (int i = 0; i < 40; ++i) {
    QApplication::processEvents();
    QThread::msleep(50);
  }

  mProjectCRS = mMapSettings3D->crs();
  return true;
}

QWidget *FlyThroughCore::findExisting3DCanvas() {
  QList<QWidget *> candidates = QApplication::topLevelWidgets();
  for (QWidget *widget : candidates) {
    if (!widget)
      continue;
    if (QString(widget->metaObject()->className()) == "Qgs3DMapCanvas") {
      return widget;
    }
    QList<QWidget *> children = widget->findChildren<QWidget *>();
    for (QWidget *child : children) {
      if (child &&
          QString(child->metaObject()->className()) == "Qgs3DMapCanvas") {
        return child;
      }
    }
  }
  return nullptr;
}

void FlyThroughCore::close3DCanvas() {
  if (mAnimTimer) {
    mAnimTimer->stop();
    mAnimTimer->deleteLater();
    mAnimTimer = nullptr;
  }

  if (mCanvas3D) {
    qDebug() << "[FTP] Closing 3D canvas...";
    mCanvas3D->close();
    mCanvas3D->deleteLater();
    mCanvas3D = nullptr;

    for (int i = 0; i < 5; ++i) {
      QApplication::processEvents();
    }
  }
}

QList<QgsPointXY> FlyThroughCore::extractPathVertices(QgsVectorLayer *layer) {
  QList<QgsPointXY> vertices;

  QgsFeatureIterator it = layer->getFeatures();
  QgsFeature feature;

  while (it.nextFeature(feature)) {
    QgsGeometry geom = feature.geometry();

    if (geom.type() == QgsWkbTypes::LineGeometry) {
      if (geom.isMultipart()) {
        QgsMultiPolylineXY multiLine = geom.asMultiPolyline();
        for (const QgsPolylineXY &line : multiLine) {
          for (const QgsPointXY &pt : line)
            vertices.append(pt);
        }
      } else {
        for (const QgsPointXY &pt : geom.asPolyline())
          vertices.append(pt);
      }
    } else if (geom.type() == QgsWkbTypes::PointGeometry) {
      if (geom.isMultipart()) {
        for (const QgsPointXY &pt : geom.asMultiPoint())
          vertices.append(pt);
      } else {
        vertices.append(geom.asPoint());
      }
    }
  }

  return vertices;
}

QList<QgsPointXY> FlyThroughCore::densifyPath(const QList<QgsPointXY> &vertices,
                                              double interval) {
  if (vertices.size() < 2)
    return vertices;

  QList<QgsPointXY> dense;
  dense.append(vertices.first());

  for (int i = 0; i < vertices.size() - 1; ++i) {
    const QgsPointXY &p1 = vertices[i];
    const QgsPointXY &p2 = vertices[i + 1];

    double dist = calculateDistance(p1, p2);
    if (dist <= interval) {
      dense.append(p2);
      continue;
    }

    int numSegments = static_cast<int>(std::ceil(dist / interval));
    double dx = (p2.x() - p1.x()) / numSegments;
    double dy = (p2.y() - p1.y()) / numSegments;

    for (int j = 1; j <= numSegments; ++j) {
      dense.append(QgsPointXY(p1.x() + dx * j, p1.y() + dy * j));
    }
  }

  return dense;
}

QList<QgsPointXY> FlyThroughCore::smoothPath(const QList<QgsPointXY> &vertices,
                                             int iterations) {
  if (iterations <= 0 || vertices.size() < 3)
    return vertices;

  QList<QgsPointXY> smoothed = vertices;

  for (int iter = 0; iter < iterations; ++iter) {
    QList<QgsPointXY> newSmoothed;
    newSmoothed.append(smoothed.first()); // Keep first

    for (int i = 1; i < smoothed.size() - 1; ++i) {
      double newX =
          (smoothed[i - 1].x() + smoothed[i].x() + smoothed[i + 1].x()) / 3.0;
      double newY =
          (smoothed[i - 1].y() + smoothed[i].y() + smoothed[i + 1].y()) / 3.0;
      newSmoothed.append(QgsPointXY(newX, newY));
    }

    newSmoothed.append(smoothed.last()); // Keep last
    smoothed = newSmoothed;
  }

  return smoothed;
}

void FlyThroughCore::generateKeyframes(const QList<QgsPointXY> &inputVertices,
                                       const FlythroughParams &params) {
  mKeyframes.clear();

  // Smooth path if requested
  QList<QgsPointXY> vertices = smoothPath(inputVertices, params.smoothing);

  // Find max elevation for "Above Safe Path" mode
  double maxElev = -9999.0;
  QList<QgsPointXY> densePoints = densifyPath(vertices, 2.0);

  for (const QgsPointXY &pt : densePoints) {
    double elev = getElevationAtPoint(params.demLayer, pt, mProjectCRS);
    if (elev > maxElev) {
      maxElev = elev;
    }
  }

  if (maxElev == -9999.0) {
    maxElev = 0.0;
  }

  double maxElevScaled = maxElev * params.verticalExaggeration;
  qDebug() << "[FTP] Path Max Elevation:" << maxElev
           << "Scaled:" << maxElevScaled;

  // Calculate fixed Z based on altitude mode
  double autoFixedZ = 0.0;
  double userFixedAMSL = 0.0;

  if (params.altitudeMode.contains("Safe Path")) {
    autoFixedZ = (maxElev + params.cameraHeight) * params.verticalExaggeration;
    qDebug() << "[FTP] Mode 'Safe Path': Fixed Z =" << autoFixedZ;
  } else if (params.altitudeMode.contains("Fixed")) {
    userFixedAMSL = params.cameraHeight * params.verticalExaggeration;
    qDebug() << "[FTP] Mode 'Fixed AMSL': Z =" << userFixedAMSL;

    if (userFixedAMSL < maxElevScaled) {
      qDebug() << "[FTP] WARNING: User AMSL lower than terrain peak!";
    }
  }

  // Generate keyframes
  double currentTime = 0.0;
  double previousBearing = 0.0;

  for (int i = 0; i < vertices.size(); ++i) {
    const QgsPointXY &point = vertices[i];

    // Get elevation
    double elevation = getElevationAtPoint(params.demLayer, point, mProjectCRS);
    double scaledElevation = elevation * params.verticalExaggeration;

    // Calculate altitude
    double targetZ = 0.0;
    if (params.altitudeMode.contains("Safe Path")) {
      targetZ = autoFixedZ;
    } else if (params.altitudeMode.contains("Fixed")) {
      targetZ = userFixedAMSL;
    } else {
      targetZ =
          scaledElevation + (params.cameraHeight * params.verticalExaggeration);
    }

    double cameraZ = targetZ;

    // Calculate yaw (heading)
    double yaw = 0.0;
    if (i < vertices.size() - 1) {
      yaw = calculateBearing(vertices[i], vertices[i + 1]);
    } else {
      yaw = previousBearing;
    }

    // Calculate banking (roll)
    double roll = 0.0;
    if (params.enableBanking && i > 0 && i < vertices.size() - 1) {
      double bearingIn = calculateBearing(vertices[i - 1], vertices[i]);
      double bearingOut = calculateBearing(vertices[i], vertices[i + 1]);

      double turnAngle = bearingOut - bearingIn;
      while (turnAngle > 180.0)
        turnAngle -= 360.0;
      while (turnAngle < -180.0)
        turnAngle += 360.0;

      roll = -turnAngle * params.bankingFactor;
      roll = qMax(-45.0, qMin(45.0, roll));
    }

    // Create keyframe
    Keyframe kf;
    kf.time = currentTime;
    kf.x = point.x();
    kf.y = point.y();
    kf.z = cameraZ;
    kf.ground_z = scaledElevation;
    kf.yaw = yaw;
    kf.pitch = params.cameraPitch;
    kf.roll = roll;

    mKeyframes.push_back(kf);

    if (i == 0 || i == vertices.size() - 1) {
      qDebug()
          << QString(
                 "[FTP] Keyframe[%1]: x=%2, y=%3, elev=%4, cam_z=%5, yaw=%6")
                 .arg(i)
                 .arg(point.x(), 0, 'f', 1)
                 .arg(point.y(), 0, 'f', 1)
                 .arg(elevation, 0, 'f', 1)
                 .arg(cameraZ, 0, 'f', 1)
                 .arg(yaw, 0, 'f', 1);
    }

    // Update time
    if (i < vertices.size() - 1) {
      double segmentDistance = calculateDistance(vertices[i], vertices[i + 1]);
      double segmentDuration = segmentDistance / params.speed;
      currentTime += segmentDuration;
    }

    previousBearing = yaw;
  }

  if (!mKeyframes.empty()) {
    mTotalDuration = mKeyframes.back().time;
  }

  qDebug() << "[FTP] Generated" << mKeyframes.size()
           << "keyframes, duration:" << mTotalDuration << "s";
}

double FlyThroughCore::getElevationAtPoint(
    QgsRasterLayer *dem, const QgsPointXY &point,
    const QgsCoordinateReferenceSystem &sourceCRS) {
  if (!dem)
    return 0.0;

  QgsCoordinateReferenceSystem demCRS = dem->crs();
  QgsPointXY samplePoint = point;

  // Transform if needed
  if (sourceCRS != demCRS) {
    QgsCoordinateTransform ct(sourceCRS, demCRS, QgsProject::instance());
    try {
      samplePoint = ct.transform(point);
    } catch (...) {
      return 0.0;
    }
  }

  // Check extent
  if (!dem->extent().contains(samplePoint)) {
    return 0.0;
  }

  // Sample raster
  QgsRasterDataProvider *provider = dem->dataProvider();
  if (!provider)
    return 0.0;

  QgsRasterIdentifyResult result =
      provider->identify(samplePoint, QgsRaster::IdentifyFormatValue);
  if (result.isValid()) {
    QMap<int, QVariant> results = result.results();
    if (results.contains(1)) {
      bool ok;
      double val = results[1].toDouble(&ok);
      if (ok)
        return val;
    }
  }

  return 0.0;
}

void FlyThroughCore::setupAnimation(const FlythroughParams &params) {
  if (mKeyframes.empty()) {
    qDebug() << "[FTP] No keyframes to animate!";
    return;
  }

  // Stop existing timer
  if (mAnimTimer) {
    mAnimTimer->stop();
    mAnimTimer->deleteLater();
  }

  // Initialize animation state
  mAnimIndex = 0;
  mAnimElapsed = 0.0;
  mAnimIntervalMs = 1000 / params.fps;
  mAnimDt = mAnimIntervalMs / 1000.0;

  qDebug() << "[FTP] Total keyframes:" << mKeyframes.size();
  qDebug() << "[FTP] Total duration:" << mTotalDuration << "s";
  qDebug() << "[FTP] FPS:" << params.fps << "Interval:" << mAnimIntervalMs
           << "ms";

  // Move to first keyframe
  const Keyframe &kf0 = mKeyframes[0];
  const Keyframe &kf1 = (mKeyframes.size() > 1) ? mKeyframes[1] : kf0;

  moveCamera(kf0.x, kf0.y, kf0.ground_z, kf0.yaw, kf0.pitch, kf1.x, kf1.y,
             kf1.ground_z, kf0.z);

  // Let tiles load
  for (int i = 0; i < 60; ++i) {
    QApplication::processEvents();
    QThread::msleep(50);
  }

  // Re-position after loading
  moveCamera(kf0.x, kf0.y, kf0.ground_z, kf0.yaw, kf0.pitch, kf1.x, kf1.y,
             kf1.ground_z, kf0.z);
  QApplication::processEvents();

  // Create timer
  mAnimTimer = new QTimer(this);
  mAnimTimer->setInterval(mAnimIntervalMs);
  connect(mAnimTimer, &QTimer::timeout, this,
          &FlyThroughCore::advanceAnimation);
  mAnimTimer->start();

  qDebug() << "[FTP] Animation timer started.";
}

void FlyThroughCore::advanceAnimation() {
  if (mKeyframes.empty() || mAnimIndex >= (int)mKeyframes.size() - 1) {
    if (mAnimTimer) {
      mAnimTimer->stop();
      qDebug() << "[FTP] Animation finished.";
    }
    return;
  }

  const Keyframe &kfA = mKeyframes[mAnimIndex];
  const Keyframe &kfB = mKeyframes[mAnimIndex + 1];
  bool hasNext = (mAnimIndex + 2) < (int)mKeyframes.size();
  const Keyframe &kfC = hasNext ? mKeyframes[mAnimIndex + 2] : kfB;

  double segDuration = kfB.time - kfA.time;
  if (segDuration <= 0)
    segDuration = 0.001;

  double localT = (mAnimElapsed - kfA.time) / segDuration;
  localT = qMax(0.0, qMin(1.0, localT));

  // Smoothstep interpolation
  double t = localT * localT * (3.0 - 2.0 * localT);

  // Interpolate position
  double x = kfA.x + (kfB.x - kfA.x) * t;
  double y = kfA.y + (kfB.y - kfA.y) * t;
  double groundZ = kfA.ground_z + (kfB.ground_z - kfA.ground_z) * t;
  double interpZ = kfA.z + (kfB.z - kfA.z) * t;

  // Interpolate angles
  double yaw = lerpAngle(kfA.yaw, kfB.yaw, t);
  double pitch = kfA.pitch + (kfB.pitch - kfA.pitch) * t;

  // Look-ahead target: smooth pan to kfC in last 20%
  double targetX, targetY, targetGz;
  if (hasNext && t > 0.8) {
    double blend = (t - 0.8) / 0.2;
    targetX = kfB.x + (kfC.x - kfB.x) * blend;
    targetY = kfB.y + (kfC.y - kfB.y) * blend;
    targetGz = kfB.ground_z + (kfC.ground_z - kfB.ground_z) * blend;
  } else {
    targetX = kfB.x;
    targetY = kfB.y;
    targetGz = kfB.ground_z;
  }

  moveCamera(x, y, groundZ, yaw, pitch, targetX, targetY, targetGz, interpZ);
  QApplication::processEvents();

  mAnimElapsed += mAnimDt;
  if (mAnimElapsed >= kfB.time) {
    mAnimIndex++;
  }
}

void FlyThroughCore::moveCamera(double x, double y, double groundZ, double yaw,
                                double pitchParam, double lookX, double lookY,
                                double lookGz, double absoluteZ) {
  if (!mCanvas3D)
    return;

  // Get camera controller via dynamic method call (avoids linking against
  // Qgs3DMapCanvas::cameraController() which was not exported in QGIS 3.28.3)
  QObject *cameraCtrl = nullptr;
  QMetaObject::invokeMethod(mCanvas3D, "cameraController", Qt::DirectConnection,
                            Q_RETURN_ARG(QObject *, cameraCtrl));
  if (!cameraCtrl) {
    // Try via scene
    QObject *scene = nullptr;
    QMetaObject::invokeMethod(mCanvas3D, "scene", Qt::DirectConnection,
                              Q_RETURN_ARG(QObject *, scene));
    if (scene) {
      QMetaObject::invokeMethod(scene, "cameraController", Qt::DirectConnection,
                                Q_RETURN_ARG(QObject *, cameraCtrl));
    }
  }
  if (!cameraCtrl)
    return;

  // Calculate look-ahead vector
  double camHeight = mCameraHeight;
  double lookAhead = mLookaheadDist;

  double dx = lookX - x;
  double dy = lookY - y;
  double rawDist = std::sqrt(dx * dx + dy * dy);

  double dxScaled = dx;
  double dyScaled = dy;
  double aheadGz = lookGz;

  if (rawDist > 1.0) {
    double scale = qMin(1.0, lookAhead / rawDist);
    dxScaled = dx * scale;
    dyScaled = dy * scale;
    aheadGz = groundZ + (lookGz - groundZ) * scale;
  } else {
    // Fallback to yaw
    double rad = qDegreesToRadians(yaw);
    dxScaled = lookAhead * std::sin(rad);
    dyScaled = lookAhead * std::cos(rad);
    aheadGz = groundZ;
  }

  double finalX = x + dxScaled;
  double finalY = y + dyScaled;

  // Sample actual terrain at look-at point
  if (mDemLayer) {
    double sampledZ =
        getElevationAtPoint(mDemLayer, QgsPointXY(finalX, finalY), mProjectCRS);
    if (sampledZ > aheadGz) {
      aheadGz = sampledZ;
    }
  }

  double camZ = absoluteZ;

  // Calculate vertical offset from pitch
  double horizM = std::sqrt(dxScaled * dxScaled + dyScaled * dyScaled);
  double pitchRad = qDegreesToRadians(pitchParam);
  double verticalOffset = horizM * std::tan(pitchRad);
  double finalZ = camZ + verticalOffset;

  // Prevent looking underground
  if (finalZ < aheadGz) {
    double vertDiff = camZ - aheadGz;
    if (vertDiff > 0 && qAbs(pitchParam) > 1.0) {
      double reqHorizM =
          vertDiff / std::tan(qDegreesToRadians(qAbs(pitchParam)));
      if (horizM > 0.1) {
        double scaleDown = reqHorizM / horizM;
        if (scaleDown < 1.0) {
          dxScaled *= scaleDown;
          dyScaled *= scaleDown;
          finalX = x + dxScaled;
          finalY = y + dyScaled;
          finalZ = aheadGz;
          horizM = reqHorizM;
        }
      }
    }
  }

  // Safety check
  if (camZ < groundZ + 1.0) {
    camZ = groundZ + 10.0;
  }

  // Calculate camera parameters
  double vert = camZ - finalZ;
  double dist = std::sqrt(horizM * horizM + vert * vert);
  if (dist < 1.0)
    dist = camHeight;

  double orbPitch =
      (horizM < 0.001) ? 0.0 : qRadiansToDegrees(std::atan2(vert, horizM));
  orbPitch = qMax(0.0, qMin(180.0, orbPitch));

  double bear = qRadiansToDegrees(std::atan2(dxScaled, dyScaled));
  double orbYaw = fmod(360.0 - bear, 360.0);

  QgsVector3D mapPt(finalX, finalY, finalZ);

  // Set camera using version-compatible API via dynamic dispatch
  int methodIdx = cameraCtrl->metaObject()->indexOfMethod(
      "setLookingAtMapPoint(QgsVector3D,double,double,double)");
  if (methodIdx >= 0) {
    QMetaObject::invokeMethod(cameraCtrl, "setLookingAtMapPoint",
                              Qt::DirectConnection, Q_ARG(QgsVector3D, mapPt),
                              Q_ARG(double, dist), Q_ARG(double, orbPitch),
                              Q_ARG(double, orbYaw));
  } else {
    // Fallback: use setCameraPose via dynamic call
    // QgsCameraPose is a simple struct - set via property or individual methods
    // Try "setViewFromTop" as ultimate fallback
    QMetaObject::invokeMethod(
        cameraCtrl, "setLookingAtPoint", Qt::DirectConnection,
        Q_ARG(QgsVector3D, mapPt), Q_ARG(float, (float)dist),
        Q_ARG(float, (float)orbPitch), Q_ARG(float, (float)orbYaw));
  }

  // Debug first few frames
  if (mDbgCount < 5) {
    mDbgCount++;
    qDebug()
        << QString(
               "[FTP] CAM #%1: pos=(%2,%3,%4) look=(%5,%6,%7) dist=%8 pitch=%9")
               .arg(mDbgCount)
               .arg(x, 0, 'f', 1)
               .arg(y, 0, 'f', 1)
               .arg(camZ, 0, 'f', 0)
               .arg(finalX, 0, 'f', 1)
               .arg(finalY, 0, 'f', 1)
               .arg(finalZ, 0, 'f', 0)
               .arg(dist, 0, 'f', 0)
               .arg(orbPitch, 0, 'f', 1);
  }
}

double FlyThroughCore::calculateBearing(const QgsPointXY &p1,
                                        const QgsPointXY &p2) const {
  double dx = p2.x() - p1.x();
  double dy = p2.y() - p1.y();
  double bearing = qRadiansToDegrees(std::atan2(dx, dy));
  return fmod(bearing + 360.0, 360.0);
}

double FlyThroughCore::calculateDistance(const QgsPointXY &p1,
                                         const QgsPointXY &p2) const {
  QgsDistanceArea da;
  da.setSourceCrs(mProjectCRS, QgsProject::instance()->transformContext());
  da.setEllipsoid(QgsProject::instance()->ellipsoid());
  return da.measureLine(p1, p2);
}

double FlyThroughCore::lerpAngle(double a, double b, double t) const {
  double diff = b - a;
  while (diff > 180.0)
    diff -= 360.0;
  while (diff < -180.0)
    diff += 360.0;
  return fmod(a + diff * t, 360.0);
}
