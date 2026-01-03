#ifndef IALARMPLUGIN_H
#define IALARMPLUGIN_H

#include <QString>
#include <QtPlugin>
#include <QWidget>

class IAlarmPlugin {
public:
    virtual ~IAlarmPlugin() = default;
    
    virtual QString pluginName() const = 0;

    // Pass a pointer to the main window UPON LOADIN
    virtual void initialize(QWidget* parent) { Q_UNUSED(parent); }

    // Called when the alarm is triggered
    virtual void trigger(const QString &title, const QMap<QString, QString> &settings) = 0;
    
    // Called to stop effects (e.g., when the window is clicked)
    virtual void stop() = 0;

    virtual QString getConfigPath() const = 0; // Example: "telegram.conf"
    virtual bool isSecurity() const = 0;       // Determines if data encryption is required
    virtual QStringList defaultSettings() const = 0;
};

#define IAlarmPlugin_iid "com.taskwidget.IAlarmPlugin/1.0"
Q_DECLARE_INTERFACE(IAlarmPlugin, IAlarmPlugin_iid)

#endif