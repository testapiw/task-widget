cmake_minimum_required(VERSION 3.16)
project(task_widget_cpp)

# Use C++17 for advanced STL features (like std::find_if and structured bindings)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enable automatic Qt meta-object compilation (MOC, UIC, and RCC)
set(CMAKE_AUTOMOC ON) 
set(CMAKE_AUTOUIC ON) 
set(CMAKE_AUTORCC ON) 

# Find required packages
find_package(Qt5 COMPONENTS Widgets Gui Core REQUIRED)
find_package(nlohmann_json REQUIRED)

# Specify header search paths
include_directories(
    src 
    src/plugin
)

# Complete list of project source files
add_executable(task_widget
    src/main.cpp
    src/UI.cpp
    src/AlarmManager.cpp
    src/PluginManager.cpp
    src/TaskDialog.cpp
    src/StyleHelper.cpp
    src/Storage.cpp
    src/plugin/PluginSettingsManager.cpp
    src/plugin/IAlarmPlugin.h
    src/plugin/PluginSettingsManager.h
    resources.qrc
)

# Configure include directories for the specific target
target_include_directories(task_widget PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/plugin 
)

# Link libraries
# ${CMAKE_DL_LIBS} is required for QPluginLoader to function correctly on Linux
target_link_libraries(task_widget 
    Qt5::Widgets 
    Qt5::Gui 
    Qt5::Core 
    nlohmann_json::nlohmann_json
    ${CMAKE_DL_LIBS}
)