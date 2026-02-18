#include "flythrough_plugin.h"
#include "flythrough_dialog.h"
#include <QAction>
#include <QIcon>
#include <QMessageBox>

// Metadata functions - must match QGIS plugin loader typedefs exactly
QString name() { return QStringLiteral("FlyThrough Pro C++"); }
QString description() {
  return QStringLiteral("C++ accelerated 3D flythrough plugin");
}
QString category() { return QStringLiteral("Plugins"); }
int type() { return QgisPlugin::UI; }

// Factory function - QGIS plugin loader calls classFactory
QgisPlugin *classFactory(QgisInterface *iface) {
  return new FlyThroughPlugin(iface);
}

// Unload function
void unload(QgisPlugin *plugin) { delete plugin; }

// Class Implementation
FlyThroughPlugin::FlyThroughPlugin(QgisInterface *iface)
    : QObject(nullptr),
      QgisPlugin(QStringLiteral("FlyThrough Pro C++"),
                 QStringLiteral("C++ accelerated 3D flythrough plugin"),
                 QStringLiteral("0.1"), QStringLiteral("Plugins"),
                 QgisPlugin::UI),
      mIface(iface) {}

FlyThroughPlugin::~FlyThroughPlugin() {
  // Cleanup handled in unload
}

void FlyThroughPlugin::initGui() {
  // Create the action
  mAction = new QAction(QIcon(), "FlyThrough Pro C++", this);
  connect(mAction, &QAction::triggered, this, &FlyThroughPlugin::run);

  // Add to QGIS UI
  mIface->addPluginToVectorMenu("FlyThrough Pro C++", mAction);
  mIface->addToolBarIcon(mAction);
}

void FlyThroughPlugin::unload() {
  mIface->removePluginVectorMenu("FlyThrough Pro C++", mAction);
  mIface->removeToolBarIcon(mAction);
  delete mAction;
}

void FlyThroughPlugin::run() {
  FlyThroughDialog dlg(mIface);
  dlg.exec();
}
