#ifndef FLYTHROUGH_CORE_H
#define FLYTHROUGH_CORE_H

#include <QObject>
#include <qgsgeometry.h>
#include <qgsvector3d.h>

class FlyThroughCore : public QObject {
  Q_OBJECT

public:
  explicit FlyThroughCore(QObject *parent = nullptr);

  // Core function to calculate camera position
  QgsVector3D calculateCameraPosition(const QgsGeometry &path, double time);
};

#endif // FLYTHROUGH_CORE_H
