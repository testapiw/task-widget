QT       += core gui widgets

TARGET = task_widget
TEMPLATE = app

CONFIG += c++17

# Header search paths
INCLUDEPATH += src \
               src/plugin

# Library for plugin support (required for Linux/Unix)
LIBS += -ldl

# Source files
SOURCES += src/main.cpp \
           src/UI.cpp \
           src/AlarmManager.cpp \
           src/PluginManager.cpp \
           src/TaskDialog.cpp \
           src/StyleHelper.cpp \
           src/Storage.cpp

# Header files
HEADERS += src/UI.h \
           src/AlarmManager.h \
           src/PluginManager.h \
           src/TaskDialog.h \
           src/StyleHelper.h \
           src/Storage.h \
           src/Task.h \
           src/plugin/IAlarmPlugin.h \
           src/plugin/PluginSettingsManager.h

# Resource files
RESOURCES += resources.qrc

# JSON settings (use if not using a system package manager)
# INCLUDEPATH += path/to/nlohmann/json