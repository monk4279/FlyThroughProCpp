#ifndef FLYTHROUGH_PLUGIN_H
#define FLYTHROUGH_PLUGIN_H

#include <QObject>
#include <QString>
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

// Standard QGIS C++ plugin entry points
// QGIS plugin loader looks for these exact function names and signatures
extern "C" {
Q_DECL_EXPORT QgisPlugin *classFactory(QgisInterface *iface);
Q_DECL_EXPORT QString name();
Q_DECL_EXPORT QString description();
Q_DECL_EXPORT QString category();
Q_DECL_EXPORT int type();
Q_DECL_EXPORT void unload(QgisPlugin *plugin);
}

#endif // FLYTHROUGH_PLUGIN_H
