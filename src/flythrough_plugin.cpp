#include "flythrough_plugin.h"
#include "flythrough_dialog.h"
#include <QAction>
#include <QIcon>
#include <QMessageBox>

// Metadata functions
const QString name() { return "FlyThrough Pro C++"; }
const QString description() { return "C++ accelerated 3D flythrough plugin"; }
const QString version() { return "0.1"; }
const QString category() { return "Plugins"; }
int type() { return QgisPlugin::UI; } // Type UI

// Factory function
FlyThroughPlugin *classFactory(QgisInterface *iface) {
  return new FlyThroughPlugin(iface);
}

// Unload function
void unload(FlyThroughPlugin *plugin) { delete plugin; }

// Class Implementation
FlyThroughPlugin::FlyThroughPlugin(QgisInterface *iface) : mIface(iface) {}

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
