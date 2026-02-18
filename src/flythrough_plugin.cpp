#include "flythrough_plugin.h"
#include "flythrough_dialog.h"
#include <QAction>
#include <QIcon>
#include <QMessageBox>

// Static strings for pointer returns (QGIS dereferences these pointers)
static const QString sName = QStringLiteral("FlyThrough Pro C++");
static const QString sDescription =
    QStringLiteral("C++ accelerated 3D flythrough plugin");
static const QString sCategory = QStringLiteral("Plugins");

// Metadata functions - return pointers to static QStrings
// Matches QGIS typedefs: typedef const QString *name_t();
const QString *name() { return &sName; }
const QString *description() { return &sDescription; }
const QString *category() { return &sCategory; }
int type() { return QgisPlugin::UI; }

// Factory function - QGIS resolves "classFactory" symbol
QgisPlugin *classFactory(QgisInterface *iface) {
  return new FlyThroughPlugin(iface);
}

// Unload function
void unload(QgisPlugin *plugin) { delete plugin; }

// Class Implementation
FlyThroughPlugin::FlyThroughPlugin(QgisInterface *iface)
    : QObject(nullptr), QgisPlugin(sName, sDescription, QStringLiteral("0.1"),
                                   sCategory, QgisPlugin::UI),
      mIface(iface) {}

FlyThroughPlugin::~FlyThroughPlugin() {}

void FlyThroughPlugin::initGui() {
  mAction = new QAction(QIcon(), "FlyThrough Pro C++", this);
  connect(mAction, &QAction::triggered, this, &FlyThroughPlugin::run);
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
