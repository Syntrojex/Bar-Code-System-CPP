cmake_minimum_required(VERSION 3.16)
project(BarcodeHub
    VERSION 0.1.0
    LANGUAGES CXX
)

# C++ Configuration
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Qt Configuration
set(CMAKE_AUTOMOC ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

# OpenCV Configuration
find_package(OpenCV REQUIRED)

# Application Sources
add_executable(BarcodeHub WIN32

    # Frontend Sources
    Frontend/src/main.cpp
    Frontend/src/icons.cpp
    Frontend/src/widgets.cpp
    Frontend/src/generatepage.cpp
    Frontend/src/decodepage.cpp
    Frontend/src/mainwindow.cpp

    # Frontend Headers
    Frontend/include/ui/theme.h
    Frontend/include/ui/icons.h
    Frontend/include/ui/widgets.h
    Frontend/include/ui/generatepage.h
    Frontend/include/ui/decodepage.h
    Frontend/include/ui/mainwindow.h

    # Backend Sources
    Backend/src/barcode/BarcodeGenerator.cpp
    Backend/src/reader/builtin.cpp
    Backend/src/reader/reader.cpp
    Backend/src/scanner/CameraScanner.cpp
    Backend/src/scanner/ImageScanner.cpp

    # Backend Headers
    Backend/include/barcode/BarcodeGenerator.h
    Backend/include/reader/builtin.h
    Backend/include/reader/reader.h
    Backend/include/scanner/CameraScanner.h
    Backend/include/scanner/ImageScanner.h
)

# Include Directories
# NOTE: Frontend/include/ui is required because the sources use flat includes
# like #include "mainwindow.h" while the headers live in Frontend/include/ui/.
target_include_directories(BarcodeHub PRIVATE
    Frontend/include
    Frontend/include/ui
    Backend/include
    ${OpenCV_INCLUDE_DIRS}
)

# Link Libraries
target_link_libraries(BarcodeHub PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    ${OpenCV_LIBS}
)

# Windows Configuration
if(WIN32)
    set_target_properties(BarcodeHub PROPERTIES
        WIN32_EXECUTABLE TRUE
    )
endif()
