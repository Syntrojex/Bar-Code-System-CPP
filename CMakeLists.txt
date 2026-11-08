cmake_minimum_required(VERSION 3.16)
project(BarcodeHub VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

# Works with Qt 6 (preferred) or Qt 5. Only the Widgets module is needed (UI only).
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

add_executable(BarcodeHub WIN32
    src/main.cpp
    src/theme.h
    src/icons.h          src/icons.cpp
    src/widgets.h        src/widgets.cpp
    src/generatepage.h   src/generatepage.cpp
    src/decodepage.h     src/decodepage.cpp
    src/mainwindow.h     src/mainwindow.cpp
)
target_link_libraries(BarcodeHub PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)
