cmake_minimum_required(VERSION 3.16)

# Skip Qt Creator's automatic vcpkg package manager injection and background maintenance tool check
set(QT_CREATOR_SKIP_VCPKG_SETUP ON CACHE BOOL "Skip Qt Creator vcpkg setup" FORCE)
set(QT_CREATOR_ENABLE_PACKAGE_MANAGER_SETUP OFF CACHE BOOL "Disable Qt Creator package manager setup" FORCE)
set(QT_CREATOR_ENABLE_MAINTENANCE_TOOL_PROVIDER OFF CACHE BOOL "Disable Qt Creator maintenance tool check" FORCE)

# Disable vcpkg manifest mode by default
if(NOT DEFINED VCPKG_MANIFEST_MODE)
    set(VCPKG_MANIFEST_MODE OFF CACHE BOOL "Disable vcpkg manifest mode by default")
endif()

# Normalize CMAKE_TOOLCHAIN_FILE path if defined
if(DEFINED CMAKE_TOOLCHAIN_FILE)
    file(TO_CMAKE_PATH "${CMAKE_TOOLCHAIN_FILE}" CMAKE_TOOLCHAIN_FILE)
elseif(DEFINED ENV{VCPKG_ROOT})
    file(TO_CMAKE_PATH "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" _vcpkg_toolchain)
    set(CMAKE_TOOLCHAIN_FILE "${_vcpkg_toolchain}" CACHE STRING "vcpkg toolchain file")
endif()

project(BarcodeHub
    VERSION 0.1.0
    LANGUAGES CXX
)

# C++ Configuration
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Qt Configuration (Qt comes from the Qt installer / Qt Creator kit, NOT from vcpkg)
set(CMAKE_AUTOMOC ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

# OpenCV Configuration (automatically detects MinGW or MSVC prebuilt packages)
if(NOT OpenCV_DIR)
    if(MINGW)
        if(EXISTS "C:/opencv-mingw/OpenCVConfig.cmake")
            set(OpenCV_DIR "C:/opencv-mingw")
        endif()
    elseif(MSVC)
        if(EXISTS "C:/opencv-msvc/OpenCVConfig.cmake")
            set(OpenCV_DIR "C:/opencv-msvc")
        elseif(EXISTS "C:/opencv/build/OpenCVConfig.cmake")
            set(OpenCV_DIR "C:/opencv/build")
        elseif(EXISTS "C:/Users/laptech/Downloads/opencv/build/OpenCVConfig.cmake")
            set(OpenCV_DIR "C:/Users/laptech/Downloads/opencv/build")
        endif()
    endif()
endif()

find_package(OpenCV QUIET)
if(OpenCV_FOUND)
    message(STATUS "OpenCV found: ${OpenCV_VERSION} (${OpenCV_DIR})")
    add_compile_definitions(HAVE_OPENCV)
else()
    message(STATUS "OpenCV not found. Building without OpenCV support (mock camera mode).")
endif()

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
)

# Link Libraries
target_link_libraries(BarcodeHub PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
)

if(OpenCV_FOUND)
    target_include_directories(BarcodeHub PRIVATE ${OpenCV_INCLUDE_DIRS})
    target_link_libraries(BarcodeHub PRIVATE ${OpenCV_LIBS})

    # Automatically copy OpenCV runtime DLLs to output directory on Windows
    if(WIN32 AND OpenCV_LIB_PATH)
        add_custom_command(TARGET BarcodeHub POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different
                "${OpenCV_LIB_PATH}/../bin"
                "$<TARGET_FILE_DIR:BarcodeHub>"
            COMMENT "Copying OpenCV runtime DLLs to output directory..."
        )
    endif()
endif()

# Windows Configuration
if(WIN32)
    set_target_properties(BarcodeHub PROPERTIES
        WIN32_EXECUTABLE TRUE
    )
endif()
