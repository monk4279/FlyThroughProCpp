#include "flythrough_plugin.h"
#include "flythrough_dialog.h"
#include <QMenu>

FlyThroughPlugin::FlyThroughPlugin(QgisInterface *iface)
    : QgisPlugin(sName, sDescription, sCategory, sVersion, sPluginType),
      mIface(iface) {}

void FlyThroughPlugin::initGui() {
  mAction =
      new QAction(QIcon(sIcon), QStringLiteral("FlyThrough Pro C++"), this);
  connect(mAction, &QAction::triggered, this, &FlyThroughPlugin::run);
  mIface->addPluginToVectorMenu(QStringLiteral("FlyThrough Pro C++"), mAction);
  mIface->addToolBarIcon(mAction);
}

void FlyThroughPlugin::unload() {
  mIface->removePluginVectorMenu(QStringLiteral("FlyThrough Pro C++"), mAction);
  mIface->removeToolBarIcon(mAction);
  delete mAction;
  mAction = nullptr;
}

void FlyThroughPlugin::run() {
  FlyThroughDialog dlg(mIface);
  dlg.exec();
}

//
// Required QGIS plugin entry points - use QGISEXTERN (same as all built-in QGIS
// plugins)
//

QGISEXTERN QgisPlugin *classFactory(QgisInterface *qgisInterfacePointer) {
  return new FlyThroughPlugin(qgisInterfacePointer);
}

QGISEXTERN const QString *name() { return &sName; }

QGISEXTERN const QString *description() { return &sDescription; }

QGISEXTERN const QString *category() { return &sCategory; }

QGISEXTERN int type() { return sPluginType; }

QGISEXTERN const QString *version() { return &sVersion; }

QGISEXTERN const QString *icon() { return &sIcon; }

QGISEXTERN void unload(QgisPlugin *pluginPointer) { delete pluginPointer; }
