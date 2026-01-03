#ifndef TELEGRAMPLUGIN_H
#define TELEGRAMPLUGIN_H

#include <QObject>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkReply>
#include <QUrlQuery>
#include <QDir>
#include <QDebug>
#include "IAlarmPlugin.h"

/**
 * @class TelegramPlugin
 * @brief Sends alarm notifications to a Telegram chat using the Telegram Bot API.
 */
class TelegramPlugin : public QObject, public IAlarmPlugin {
    Q_OBJECT
    Q_INTERFACES(IAlarmPlugin)
    Q_PLUGIN_METADATA(IID IAlarmPlugin_iid)

public:
    QString pluginName() const override { return "Telegram Notifier"; }
    
    QString getConfigPath() const override { 
        return QDir::homePath() + "/.local/share/tasks/TelegramPlugin.conf"; 
    }
    
    // Default keys required in the configuration file
    QStringList defaultSettings() const override { return {"token", "chat_id"}; }
    
    // Data should be encrypted as it contains a sensitive bot token
    bool isSecurity() const override { return true; }

    void initialize(QWidget* parent) override {
        Q_UNUSED(parent);
        // qDebug() << "[TelegramPlugin] Initialized";
    }

    /**
     * @brief Sends an HTTP POST request to the Telegram Bot API.
     */
    void trigger(const QString &title, const QMap<QString, QString> &settings) override {
        QString botToken = settings.value("token");
        QString chatId = settings.value("chat_id");

        // qDebug() << "[TelegramPlugin] Triggered with title:" << title;

        if (botToken.isEmpty() || chatId.isEmpty()) {
            qWarning() << "[TelegramPlugin] Error: Token or ChatID is missing in settings!";
            return;
        }

        auto *manager = new QNetworkAccessManager(this);
        QUrl url(QString("https://api.telegram.org/bot%1/sendMessage").arg(botToken));
        QNetworkRequest request(url);
        request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");

        QUrlQuery params;
        params.addQueryItem("chat_id", chatId);
        params.addQueryItem("text", "🔔 TaskWidget: " + title);

        // qDebug() << "[TelegramPlugin] Sending request to Telegram API...";

        QNetworkReply *reply = manager->post(request, params.toString(QUrl::FullyEncoded).toUtf8());

        // Log the result of the network request
        connect(reply, &QNetworkReply::finished, [reply, manager]() {
            if (reply->error() == QNetworkReply::NoError) {
                // qDebug() << "[TelegramPlugin] Success! Message sent. Response:" << reply->readAll();
            } else {
                // qCritical() << "[TelegramPlugin] Network Error:" << reply->errorString();
                // qCritical() << "[TelegramPlugin] HTTP Code:" << reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
                // qCritical() << "[TelegramPlugin] Full Response:" << reply->readAll();
            }
            
            // Clean up resources after the request is finished
            reply->deleteLater();
            manager->deleteLater();
        });
    }

    void stop() override {
        // qDebug() << "[TelegramPlugin] Stop called";
    }
};

#endif // TELEGRAMPLUGIN_H