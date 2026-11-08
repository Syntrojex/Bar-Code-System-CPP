# BarcodeHub – Qt C++ UI (front end only)

Build (Qt 6 or Qt 5, Widgets only):

    cmake -B build -DCMAKE_BUILD_TYPE=Release
    cmake --build build
    ./build/BarcodeHub

Or open CMakeLists.txt in Qt Creator.

Screens (see preview/): Generate Barcode, Decode > Scan with Camera, Upload Image, Scan Logs.
Optional fonts for the exact look: Inter and JetBrains Mono (falls back to system fonts).

Backend hooks (signals/slots), nothing is implemented behind them:
  GeneratePage : generateRequested(code) / setBarcodeImage(pixmap)
  DecodePage   : cameraStartRequested(i), cameraStopRequested(), decodeImageRequested(path)
                 showResult(code), showFailure(), setCameraList(names), cameraView()->setFrame(pixmap)
Placeholders: barcode bars are decorative; "Decode Image" in mainwindow.cpp just toggles success/failure (marked PLACEHOLDER).
