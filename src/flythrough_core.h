#ifndef FLYTHROUGH_CORE_H
#define FLYTHROUGH_CORE_H

#include <QMap>
#include <QObject>
#include <QTimer>
#include <qgis.h>
#include <qgscoordinatereferencesystem.h>
#include <qgsgeometry.h>
#include <qgsmaplayer.h>
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
class QgisInterface;
class QgsCameraController;

struct Keyframe {
  double time;     // Seconds from start
  double x, y;     // Map coordinates (View CRS)
  double z;        // Absolute camera altitude
  double ground_z; // Terrain elevation at this point
  double yaw;      // Heading (0-360°)
  double pitch;    // Look angle (-90 to 90°)
  double roll;     // Banking angle (±45°)
};

struct FlythroughParams {
  QgsVectorLayer *pathLayer = nullptr;
  QgsRasterLayer *demLayer = nullptr;
  QgsMapLayer *overlayLayer = nullptr;

  QString altitudeMode = "Above Safe Path"; // or "Fixed Altitude (AMSL)"
  double cameraHeight = 200.0;              // meters
  double cameraPitch = 65.0;                // degrees (positive = down)
  double fieldOfView = 45.0;                // degrees
  double verticalExaggeration = 1.0;
  double speed = 50.0; // m/s
  int smoothing = 0;   // iterations
  bool enableBanking = true;
  double bankingFactor = 0.5;
  bool terrainShading = true;
  double lookaheadDistance = 1000.0; // meters
  int fps = 30;
};

class FlyThroughCore : public QObject {
  Q_OBJECT

public:
  explicit FlyThroughCore(QgisInterface *iface, QObject *parent = nullptr);
  ~FlyThroughCore();

  // Main generation function
  bool generateFlythrough(const FlythroughParams &params);

  // Stop animation
  void stopAnimation();

private:
  QgisInterface *mIface;
  Qgs3DMapCanvas *mCanvas3D = nullptr;
  Qgs3DMapSettings *mMapSettings3D = nullptr;
  QgsRasterLayer *mDemLayer = nullptr;
  QgsCoordinateReferenceSystem mProjectCRS;

  std::vector<Keyframe> mKeyframes;
  double mTotalDuration = 0.0;
  double mCameraHeight = 200.0;
  double mLookaheadDist = 1000.0;
  double mPitchAngle = -65.0;
  double mVerticalScale = 1.0;

  // Animation state
  QTimer *mAnimTimer = nullptr;
  int mAnimIndex = 0;
  double mAnimElapsed = 0.0;
  double mAnimDt = 0.0;
  int mAnimIntervalMs = 33;
  int mDbgCount = 0;

  // Methods
  bool setup3DCanvas(const FlythroughParams &params,
                     const QgsPointXY &startPoint);
  Qgs3DMapCanvas *findExisting3DCanvas();
  void close3DCanvas();

  QList<QgsPointXY> extractPathVertices(QgsVectorLayer *layer);
  QList<QgsPointXY> densifyPath(const QList<QgsPointXY> &vertices,
                                double interval);
  QList<QgsPointXY> smoothPath(const QList<QgsPointXY> &vertices,
                               int iterations);

  void generateKeyframes(const QList<QgsPointXY> &vertices,
                         const FlythroughParams &params);
  double getElevationAtPoint(QgsRasterLayer *dem, const QgsPointXY &point,
                             const QgsCoordinateReferenceSystem &sourceCRS);

  void setupAnimation(const FlythroughParams &params);
  void moveCamera(double x, double y, double groundZ, double yaw,
                  double pitchParam, double lookX, double lookY, double lookGz,
                  double absoluteZ);

  // Math helpers
  double calculateBearing(const QgsPointXY &p1, const QgsPointXY &p2) const;
  double calculateDistance(const QgsPointXY &p1, const QgsPointXY &p2) const;
  double lerpAngle(double a, double b, double t) const;

private slots:
  void advanceAnimation();
};

#endif // FLYTHROUGH_CORE_H
