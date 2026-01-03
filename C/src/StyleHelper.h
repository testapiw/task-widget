#pragma once
#include <QString>

/**
 * @class StyleHelper
 * @brief Centralized utility for managing the application's visual themes (QSS).
 * * This class provides static methods to generate Qt Style Sheets (QSS). 
 * Centralizing styles here ensures visual consistency across the UI and 
 * simplifies the process of making global design changes.
 */
class StyleHelper {
public:
    /**
     * @brief Returns the main window stylesheet.
     * @param isAlarm If true, returns a style with an alert highlight (e.g., red borders).
     */
    static QString getMainStyle(bool isAlarm = false);

    /**
     * @brief Generates a stylesheet for status buttons with a dynamic background.
     * @param color The hex or named color to apply to the button.
     */
    static QString getStatusStyle(const QString& color);

    /**
     * @brief Returns the standard stylesheet for custom dialog boxes.
     */
    static QString getDialogStyle();

    // Helper color constants for design uniformity
    static QString colorBackground() { return "#1e1e1e"; }
    static QString colorAccent()     { return "#3498db"; }
    static QString colorDanger()     { return "#ff4757"; }
};