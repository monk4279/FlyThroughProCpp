#ifndef FLYTHROUGH_CORE_H
#define FLYTHROUGH_CORE_H

#include <QObject>
#include <qgis.h>
#include <qgsgeometry.h>
#include <qgspointxy.h>
#include <qgsraster.h>
#include <qgsrectangle.h>
#include <qgsvector3d.h>
#include <vector>

// Forward declarations
class QgsVectorLayer;
class QgsRasterLayer;
class Qgs3DMapSettings;
class Qgs3DMapCanvas;

struct CameraKeyframe {
  double time;
  QgsVector3D position; // x, y, z
  double yaw;
  double pitch;
  double roll;
};

class FlyThroughCore : public QObject {
  Q_OBJECT

public:
  explicit FlyThroughCore(QObject *parent = nullptr);

  // Main generation function
  bool generateKeyframes(QgsVectorLayer *pathLayer, QgsRasterLayer *demLayer,
                         double speedKmh, double altitudeOffset, double pitch,
                         bool useBanking);

  // Get interpolated camera state
  CameraKeyframe interpolate(double time) const;

  // Get total duration
  double totalDuration() const { return mTotalDuration; }

  // Access keyframes
  const std::vector<CameraKeyframe> &keyframes() const { return mKeyframes; }

private:
  std::vector<CameraKeyframe> mKeyframes;
  double mTotalDuration = 0.0;

  // Helper to densify path
  QgsGeometry densifyPath(const QgsGeometry &geom, double interval);

  // Helper to sample elevation
  double getElevation(QgsRasterLayer *dem, const QgsPointXY &point);

  // Math helpers
  double calculateBearing(const QgsPointXY &p1, const QgsPointXY &p2) const;
  double lerpAngle(double a, double b, double t) const;
};

#endif // FLYTHROUGH_CORE_H
