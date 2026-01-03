#include "StyleHelper.h"

QString StyleHelper::getMainStyle(bool isAlarm) {
    // The main window's outer border flashes only during a global alarm state
    QString borderColor = isAlarm ? "#ff4757" : "#333";

    return QString(R"(
        #centralWidget { 
            background-color: rgba(30, 30, 30, 235); 
            border: 1px solid %1; 
            border-radius: 12px; 
        }
        #dragHandle { color: #888; background: rgba(60, 60, 60, 100); padding: 4px 10px; border-radius: 4px; }
        #btnNew { background: #333; color: #efefef; border: 1px solid #444; padding: 6px; border-radius: 4px; }
        #btnNew:hover { background: #444; }
        
        QTreeWidget { background: transparent; border: none; color: #efefef; outline: 0; }
        
        QTreeWidget::item { padding: 4px; border-bottom: 1px solid rgba(255,255,255,5); }
        QTreeWidget::item:selected { 
            background: rgba(52, 152, 219, 40); 
            border-left: 3px solid #3498db; 
            color: white; 
        }
        QTreeWidget::item:hover { background: rgba(255, 255, 255, 15); }

        QScrollBar:vertical { border: none; background: transparent; width: 6px; }
        QScrollBar::handle:vertical { background: #444; border-radius: 3px; }
    )").arg(borderColor);
}

QString StyleHelper::getStatusStyle(const QString& color) {
    return QString("QPushButton { "
        "   color: %1; background: transparent; border: none; "
        "   font-size: 18px; font-weight: bold; "
        "   padding-bottom: 2px; "
        "} "
        "QPushButton:hover { background: rgba(255,255,255,10); border-radius: 12px; }").arg(color);
}

QString StyleHelper::getDialogStyle() {
    return R"(
        QDialog { background-color: #1e1e1e; }
        QLabel { color: #aaa; }
        QLineEdit, QTimeEdit { background: #2a2a2a; color: white; border: 1px solid #333; padding: 5px; border-radius: 4px; }
        QPushButton#dayBtn { background: #2a2a2a; color: #888; border-radius: 15px; }
        QPushButton#dayBtn:checked { background: #3498db; color: white; }
        QPushButton#btnSave { background: #2ecc71; color: white; padding: 8px; font-weight: bold; border-radius: 4px; }
    )";
}