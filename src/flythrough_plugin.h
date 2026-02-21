#ifndef FLYTHROUGH_PLUGIN_H
#define FLYTHROUGH_PLUGIN_H

#include "qgis.h"
#include "qgisplugin.h"
#include <QAction>
#include <QApplication>

class QgisInterface;

class FlyThroughPlugin : public QObject, public QgisPlugin {
  Q_OBJECT

public:
  explicit FlyThroughPlugin(QgisInterface *iface);
  void initGui() override;
  void unload() override;

public slots:
  void run();

private:
  QgisInterface *mIface = nullptr;
  QAction *mAction = nullptr;
};

// Static metadata strings (matching QGIS plugin pattern)
static const QString sName = QStringLiteral("FlyThrough Pro C++");
static const QString sDescription =
    QStringLiteral("C++ accelerated 3D flythrough plugin");
static const QString sCategory = QStringLiteral("Plugins");
static const QString sVersion = QStringLiteral("0.1");
static const QString sIcon = QStringLiteral("");
static const QgisPlugin::PluginType sPluginType = QgisPlugin::UI;

#endif // FLYTHROUGH_PLUGIN_H
