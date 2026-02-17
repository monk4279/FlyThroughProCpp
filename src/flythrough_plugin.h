#ifndef FLYTHROUGH_PLUGIN_H
#define FLYTHROUGH_PLUGIN_H

#include <QObject>
#include <qgisinterface.h>
#include <qgisplugin.h>

class FlyThroughPlugin : public QObject, public QgisPlugin {
  Q_OBJECT

public:
  explicit FlyThroughPlugin(QgisInterface *iface);
  ~FlyThroughPlugin() override;

  void initGui() override;
  void unload() override;

public slots:
  void run();

private:
  QgisInterface *mIface = nullptr;
  QAction *mAction = nullptr;
};

// Standard QGIS plugin entry points
extern "C" {
Q_DECL_EXPORT void unload(FlyThroughPlugin *plugin);
Q_DECL_EXPORT FlyThroughPlugin *classFactory(QgisInterface *iface);
}

#endif // FLYTHROUGH_PLUGIN_H
