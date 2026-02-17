#ifndef FLYTHROUGH_PLUGIN_H
#define FLYTHROUGH_PLUGIN_H

#include <QObject>
#include <qgisplugin.h>
#include <qgisinterface.h>

class FlyThroughPlugin : public QObject
{
    Q_OBJECT

public:
    explicit FlyThroughPlugin(QgisInterface *iface);
    ~FlyThroughPlugin();

    void initGui();
    void unload();

public slots:
    void run();

private:
    QgisInterface *mIface = nullptr;
    QAction *mAction = nullptr;
};

// Standard QGIS plugin entry points
extern "C" {
    Q_DECL_EXPORT const QString name();
    Q_DECL_EXPORT const QString description();
    Q_DECL_EXPORT const QString version();
    Q_DECL_EXPORT const QString category();
    Q_DECL_EXPORT int type();
    Q_DECL_EXPORT void unload(FlyThroughPlugin* plugin);
    Q_DECL_EXPORT FlyThroughPlugin* classFactory(QgisInterface *iface);
}

#endif // FLYTHROUGH_PLUGIN_H
