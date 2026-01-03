#ifndef BLINKPLUGIN_H
#define BLINKPLUGIN_H

#include <QObject>
#include <QTimer>
#include <QPointer>
#include <QWidget>
#include "IAlarmPlugin.h"

/**
 * @class BlinkPlugin
 * @brief A plugin that creates a visual blinking effect on the main window border.
 */
class BlinkPlugin : public QObject, public IAlarmPlugin {
    Q_OBJECT
    Q_INTERFACES(IAlarmPlugin)
    Q_PLUGIN_METADATA(IID IAlarmPlugin_iid)

public:
    BlinkPlugin() {
        m_timer = new QTimer(this);
        connect(m_timer, &QTimer::timeout, this, &BlinkPlugin::toggle);
    }

    QString pluginName() const override { return "Border Blinker"; }
    
    // Store the pointer to the main window once during initial loading
    void initialize(QWidget* parent) override { m_window = parent; }

    /**
     * @brief Starts the blinking effect.
     */
    void trigger(const QString &title, const QMap<QString, QString> &settings) override {
        Q_UNUSED(title); Q_UNUSED(settings);
        if (!m_timer->isActive()) m_timer->start(500); // 500ms interval
    }

    /**
     * @brief Stops the effect and restores the default style.
     */
    void stop() override {
        m_timer->stop();
        updateStyle(false);
    }

    // Configuration stubs (not used by this specific plugin)
    QString getConfigPath() const override { return ""; }
    bool isSecurity() const override { return false; }
    QStringList defaultSettings() const override { return {}; }

private slots:
    /**
     * @brief Toggles the visual state (on/off) on each timer tick.
     */
    void toggle() {
        m_state = !m_state;
        updateStyle(m_state);
    }

private:
    /**
     * @brief Updates the stylesheet of the central widget.
     * @param active If true, applies the alert color; otherwise, applies the default.
     */
    void updateStyle(bool active) {
        if (m_window.isNull()) return;
        
        // Searching for the central widget by object name
        QWidget* central = m_window->findChild<QWidget*>("centralWidget");
        if (central) {
            QString color = active ? "#ff4757" : "#333";
            central->setStyleSheet(QString("#centralWidget { border: 2px solid %1; border-radius: 12px; }").arg(color));
        }
    }

    QTimer* m_timer;
    QPointer<QWidget> m_window; // Use QPointer to safely handle window deletion
    bool m_state = false;
};

#endif // BLINKPLUGIN_H