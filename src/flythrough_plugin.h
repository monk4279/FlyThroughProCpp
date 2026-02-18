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
// Exact typedefs from qgspluginregistry.cpp:
//   typedef QgisPlugin *create_ui( QgisInterface *qI );
//   typedef const QString *name_t();
//   typedef const QString *description_t();
//   typedef const QString *category_t();
//   typedef int type_t();
extern "C" {
Q_DECL_EXPORT QgisPlugin *classFactory(QgisInterface *iface);
Q_DECL_EXPORT const QString *name();
Q_DECL_EXPORT const QString *description();
Q_DECL_EXPORT const QString *category();
Q_DECL_EXPORT int type();
Q_DECL_EXPORT void unload(QgisPlugin *plugin);
}

#endif // FLYTHROUGH_PLUGIN_H
