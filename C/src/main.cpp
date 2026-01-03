#include <QApplication>
#include <QMetaType>
#include "UI.h"

int main(int argc, char *argv[]) {

    qputenv("QT_QPA_PLATFORM", "xcb");
    qRegisterMetaType<Task>("Task");

    QApplication app(argc, argv);
    
    app.setApplicationName("TaskWidget");
    app.setOrganizationName("DevTools");


    app.setStyleSheet(
        "QWidget { font-family: 'Segoe UI', 'Ubuntu', sans-serif; font-size: 13px; }"
        "QToolTip { color: #ffffff; background-color: #2a2a2a; border: 1px solid #444; }"
    );

    MainWindow w;
    w.show();

    return app.exec();
}