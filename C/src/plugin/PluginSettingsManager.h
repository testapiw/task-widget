#ifndef PLUGINSETTINGSMANAGER_H
#define PLUGINSETTINGSMANAGER_H

#include <QString>
#include <QMap>
#include <QStringList>
#include <QSettings>
#include <QFile>
#include <QDir>
#include <QCryptographicHash>

/**
 * @class PluginSettingsManager
 * @brief Handles configuration file operations with optional data encryption.
 */
class PluginSettingsManager {
public:
    /**
     * @brief Encrypts or decrypts data using a hash-based stream cipher.
     * @param data The input string.
     * @param encrypt Set to true for encryption (returns Base64), false for decryption (expects Base64).
     * @return The processed string.
     */
    static QString encryptDecrypt(const QString &data, bool encrypt);

    /**
     * @brief Loads all settings from a configuration file.
     * @param path File system path to the .ini file.
     * @param secure If true, values will be decrypted during loading.
     * @return A map of key-value pairs.
     */
    static QMap<QString, QString> loadSettings(const QString &path, bool secure);

    /**
     * @brief Saves a single key-value pair to the configuration file.
     * @param path File system path to the .ini file.
     * @param key The setting name.
     * @param value The value to store.
     * @param secure If true, the value will be encrypted before storage.
     */
    static void saveSetting(const QString &path, const QString &key, const QString &value, bool secure);

    /**
     * @brief Ensures the config file and directory exist, initializing default keys if missing.
     * @param path File system path to the .ini file.
     * @param keys List of keys to create if the file is new.
     * @param secure If true, initializes keys using the encryption format.
     */
    static void initShortcut(const QString &path, const QStringList &keys, bool secure);
};

#endif // PLUGINSETTINGSMANAGER_H