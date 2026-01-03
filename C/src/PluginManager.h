#pragma once
#include <QObject>
#include <QStringList>
#include <QDir>
#include <QPluginLoader>
#include "IAlarmPlugin.h"
#include <QSettings>

/**
 * @class PluginManager
 * @brief Manages the dynamic loading and lifecycle of alarm notification plugins.
 * * This class scans for external shared libraries (plugins) that implement the IAlarmPlugin 
 * interface, allowing the application to extend its notification capabilities (e.g., 
 * playing sounds, showing desktop notifications, or controlling hardware) without 
 * modifying the core code.
 */
class PluginManager : public QObject {
    Q_OBJECT
public:
    explicit PluginManager(QObject *parent = nullptr);
    ~PluginManager();

    /**
     * @brief Discovers and loads all plugins from the standard application path.
     * @param mainWindow A pointer to the main window, often required for UI-based plugins.
     */
    void loadPlugins(QWidget* mainWindow);
    
    /**
     * @brief Activates all loaded plugins simultaneously.
     * @param message The alert message to be displayed or processed by plugins.
     * @param parent The parent widget for any plugin-generated dialogs.
     */
    void triggerAll(const QString &message, QWidget *parent);
    
    /**
     * @brief Stops all active plugin actions (e.g., silences audio or hides overlays).
     */
    void stopAll();

    /**
     * @brief Returns a list of names for all successfully loaded plugins.
     * @return QStringList containing plugin identifiers for logging or settings.
     */
    QStringList loadedPluginNames() const;

    /** @brief Provides direct access to the list of loaded plugin interfaces. */
    QList<IAlarmPlugin*> getPlugins() { return m_plugins; }

private:
    QList<IAlarmPlugin*> m_plugins; // Interfaces used to interact with the plugins
    QList<QPluginLoader*> m_loaders; // Stored loaders to ensure safe unloading in the destructor
};