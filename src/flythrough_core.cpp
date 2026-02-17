#include "flythrough_core.h"

FlyThroughCore::FlyThroughCore(QObject *parent) : QObject(parent) {}

QgsVector3D FlyThroughCore::calculateCameraPosition(const QgsGeometry &path,
                                                    double time) {
  // Placeholder logic
  return QgsVector3D(0, 0, 0);
}
