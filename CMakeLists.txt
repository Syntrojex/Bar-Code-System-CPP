cmake_minimum_required(VERSION 3.21)
project(BarcodeHub VERSION 0.1.0 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_AUTOMOC ON)

# Qt, OpenCV and the application must use the same compiler ABI.
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)
find_package(OpenCV 4 REQUIRED COMPONENTS core imgproc imgcodecs videoio)
message(STATUS "OpenCV ${OpenCV_VERSION}: ${OpenCV_DIR}")

add_library(barcode_core STATIC
    Backend/src/barcode/BarcodeGenerator.cpp
    Backend/src/barcode/Decoder.cpp
    Backend/src/reader/builtin.cpp
    Backend/src/reader/reader.cpp
    Backend/src/scanner/CameraScanner.cpp
    Backend/src/scanner/ImageScanner.cpp
    Backend/src/scanner/Scanner.cpp
)
target_include_directories(barcode_core PUBLIC Backend/include
    Backend/include/barcode Backend/include/scanner ${OpenCV_INCLUDE_DIRS})
target_compile_definitions(barcode_core PUBLIC HAVE_OPENCV)
target_link_libraries(barcode_core PUBLIC Qt${QT_VERSION_MAJOR}::Widgets ${OpenCV_LIBS})
if(WIN32)
    target_link_libraries(barcode_core PUBLIC ole32 oleaut32 strmiids)
endif()

add_library(barcode_ui STATIC
    Frontend/src/icons.cpp Frontend/src/widgets.cpp
    Frontend/src/generatepage.cpp Frontend/src/decodepage.cpp
    Frontend/src/mainwindow.cpp
    Frontend/include/ui/icons.h Frontend/include/ui/widgets.h
    Frontend/include/ui/generatepage.h Frontend/include/ui/decodepage.h
    Frontend/include/ui/mainwindow.h Frontend/include/ui/theme.h
)
target_include_directories(barcode_ui PUBLIC Frontend/include/ui)
target_link_libraries(barcode_ui PUBLIC barcode_core)
add_executable(BarcodeHub WIN32 Frontend/src/main.cpp)
target_link_libraries(BarcodeHub PRIVATE barcode_ui)

function(copy_runtime target)
    if(WIN32)
        add_custom_command(TARGET ${target} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                $<TARGET_RUNTIME_DLLS:${target}> $<TARGET_FILE_DIR:${target}>
            COMMAND_EXPAND_LISTS VERBATIM)
    endif()
endfunction()
copy_runtime(BarcodeHub)

include(CTest)
if(BUILD_TESTING)
    add_executable(barcode_tests tests/regression.cpp)
    target_link_libraries(barcode_tests PRIVATE barcode_ui)
    copy_runtime(barcode_tests)
    add_test(NAME barcode_regression COMMAND barcode_tests)
    set_tests_properties(barcode_regression PROPERTIES
        ENVIRONMENT "QT_QPA_PLATFORM=offscreen" TIMEOUT 90)
endif()
