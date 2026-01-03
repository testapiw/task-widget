QT += network widgets
TEMPLATE = lib
CONFIG += plugin

PLUGIN_NAME = blink_plugin
TARGET = $$PLUGIN_NAME

# --- DIRECTORY STRUCTURE ---

DESTDIR = desc/$$PLUGIN_NAME

OBJECTS_DIR = build/$$PLUGIN_NAME/obj
MOC_DIR     = build/$$PLUGIN_NAME/moc
RCC_DIR     = build/$$PLUGIN_NAME/rcc
UI_DIR      = build/$$PLUGIN_NAME/ui

# --- PATHS ---
HEADERS += src/BlinkPlugin.h
# SOURCES += src/BlinkPlugin.cpp 

INCLUDEPATH += $$PWD/../../C/src/plugin

# --- INSTALLATION ---
target.path = $$(HOME)/.local/share/tasks/plugins
INSTALLS += target