#include "PluginManager.h"
#include "PluginSettingsManager.h"
#include <QDir>
#include <QDebug>

PluginManager::PluginManager(QObject *parent) : QObject(parent) {}

PluginManager::~PluginManager() {
    stopAll();
    for (auto *loader : m_loaders) {
        loader->unload();
        delete loader;
    }
}

void PluginManager::loadPlugins(QWidget* mainWindow) {
    QString pluginsPath = QDir::homePath() + "/.local/share/tasks/plugins";
    QDir dir(pluginsPath);

    if (!dir.exists()) {
        dir.mkpath(".");
        return;
    }

    for (const QString &fileName : dir.entryList({"*.so", "*.dll"}, QDir::Files)) {
        QString fullPath = dir.absoluteFilePath(fileName);
        QPluginLoader *loader = new QPluginLoader(fullPath, this);
        
        if (QObject *instance = loader->instance()) {
            if (auto *plugin = qobject_cast<IAlarmPlugin*>(instance)) {
                plugin->initialize(mainWindow);
                m_plugins.append(plugin);
                m_loaders.append(loader);
            } else {
                loader->unload();
                delete loader;
            }
        } else {
            delete loader;
        }
    }
}

/**
 * @brief Activates all enabled plugins and passes their specific configuration.
 * * This method iterates through loaded plugins, checks their activation status in 
 * global settings, decrypts their specific configurations, and triggers their action.
 * * @param message The alert message to be processed (e.g., the task name).
 * @param parent Unused parameter (retained for signature compatibility).
 */
void PluginManager::triggerAll(const QString &message, QWidget *parent) {
    Q_UNUSED(parent); // Parent is no longer needed by plugins for settings
    QSettings globalSettings("TaskWidget", "Plugins");

    for (auto *p : m_plugins) {
        if (!p) continue;
        // 1. Check if the plugin is enabled via the settings checkbox
        if (globalSettings.value(p->pluginName(), true).toBool()) { 
            
            // 2. RETRIEVE AND DECRYPT SETTINGS HERE
            // The main application manages paths and decryption methods    
            QMap<QString, QString> decryptedSettings = PluginSettingsManager::loadSettings(
                p->getConfigPath(), 
                p->isSecurity()
            );

            // 3. Pass only the ready-to-use results to the plugin
            p->trigger(message, decryptedSettings);
        }
    }
}

void PluginManager::stopAll() {
    for (auto *plugin : m_plugins) { plugin->stop(); }
}

