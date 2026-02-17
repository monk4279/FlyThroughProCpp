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
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(                                                               \
    disable : 4190) // Disable "C-linkage returning C++ object" warning
#endif

extern "C" {
Q_DECL_EXPORT const QString name();
Q_DECL_EXPORT const QString description();
Q_DECL_EXPORT const QString version();
Q_DECL_EXPORT const QString category();
Q_DECL_EXPORT int type();
Q_DECL_EXPORT void unload(FlyThroughPlugin *plugin);
Q_DECL_EXPORT FlyThroughPlugin *classFactory(QgisInterface *iface);
}

#ifdef _MSC_VER
#pragma warning(pop)
#endif

#endif // FLYTHROUGH_PLUGIN_H
