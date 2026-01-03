#include "PluginSettingsManager.h"
#include <QFileInfo>
#include <QDir>
#include <QFile>
#include <QCryptographicHash>

/**
 * @brief Simple stream cipher using SHA-256 and Base64.
 * @param data Input text.
 * @param encrypt True to encrypt and encode to Base64, false to decode Base64 and decrypt.
 */
QString PluginSettingsManager::encryptDecrypt(const QString &data, bool encrypt) {
    if (data.isEmpty()) return data;

    // A fixed key used to generate a 32-byte hash
    QByteArray keyHash = QCryptographicHash::hash("Global_TaskWidget_Key_2026", QCryptographicHash::Sha256);
    
    if (encrypt) {
        QByteArray bytes = data.toUtf8();
        for (int i = 0; i < bytes.size(); ++i) {
            bytes[i] = bytes[i] ^ keyHash[i % keyHash.size()];
        }
        return QString::fromLatin1(bytes.toBase64());
    } else {
        QByteArray bytes = QByteArray::fromBase64(data.toLatin1());
        for (int i = 0; i < bytes.size(); ++i) {
            bytes[i] = bytes[i] ^ keyHash[i % keyHash.size()];
        }
        return QString::fromUtf8(bytes);
    }
}

/**
 * @brief Loads settings from the INI file, automatically decrypting secure fields.
 */
QMap<QString, QString> PluginSettingsManager::loadSettings(const QString &path, bool secure) {
    QMap<QString, QString> data;
    QSettings settings(path, QSettings::IniFormat);

    for (const QString &key : settings.allKeys()) {
        QString val = settings.value(key).toString();
        data[key] = (secure && !val.isEmpty()) ? encryptDecrypt(val, false) : val;
    }
    return data;
}

/**
 * @brief Saves a setting, encrypting it first if secure is enabled.
 */
void PluginSettingsManager::saveSetting(const QString &path, const QString &key, const QString &value, bool secure) {
    QSettings settings(path, QSettings::IniFormat);
    QString finalValue = (secure && !value.isEmpty()) ? encryptDecrypt(value, true) : value;
    settings.setValue(key, finalValue);
    settings.sync(); 
}

/**
 * @brief Ensures the config directory exists and creates default keys.
 */
void PluginSettingsManager::initShortcut(const QString &path, const QStringList &keys, bool secure) {
    QFileInfo info(path);
    if (!info.dir().exists()) {
        info.dir().mkpath(".");
    }

    if (!QFile::exists(path)) {
        QSettings settings(path, QSettings::IniFormat);
        for (const QString &key : keys) {
            // We initialize with an empty but "valid" format
            settings.setValue(key, "");
        }
        settings.sync();
    }
}