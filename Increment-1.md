# Team Task Distribution
## Bar-Code-System-CPP — Increment 1

### 1. Muhammad Mustafa Amir (Syntrojex) — Project Lead

**Module: Application Structure, GUI & Integration**

**Responsibilities:**
- Set up CMake and project dependencies.
- Establish the main application structure.
- Develop the main GUI and screen navigation using SFML.
- Integrate all team members' modules.
- Review Pull Requests and test the complete application.

**Files to Create:**
```text
src/
└── main.cpp
└── app/
    └── Application.cpp
└── ui/
    ├── MainMenu.cpp
    ├── GeneratorScreen.cpp
    └── ScannerScreen.cpp

include/
├── app/
│   └── Application.hpp
└── ui/
    ├── MainMenu.hpp
    ├── GeneratorScreen.hpp
    └── ScannerScreen.hpp

CMakeLists.txt
CMakePresets.json
vcpkg.json
```

### 2. Ali — Barcode Generation

**Module: Barcode Generator**

**Responsibilities:**
- Accept six-digit numeric input.
- Validate user input.
- Generate barcodes using ZXing-C++.
- Return the generated barcode image.
- Handle invalid input and generation errors.

**Files to Create:**
```text
src/
└── barcode/
    ├── BarcodeGenerator.cpp
    └── Validator.cpp

include/
└── barcode/
    ├── BarcodeGenerator.hpp
    └── Validator.hpp
```

### 3. Farzam — Image-Based Barcode Scanning

**Module: Image Scanner**

**Responsibilities:**
- Allow users to select and upload barcode images.
- Load and process selected images.
- Decode barcodes using ZXing-C++.
- Extract and return barcode data.
- Handle invalid or unreadable images.

**Files to Create:**
```text
src/
└── scanner/
    └── ImageScanner.cpp

include/
└── scanner/
    └── ImageScanner.hpp
```

### 4. Shozab — OpenCV & Camera-Based Barcode Scanning

**Module: Live Camera Scanner**

**Responsibilities:**
- Configure and integrate OpenCV for camera access.
- Capture live camera input.
- Process camera frames.
- Display the live camera feed through the application.
- Decode detected barcodes using ZXing-C++.
- Extract and return barcode data.
- Handle camera access and connection errors.

**Files to Create:**
```text
src/
└── scanner/
    └── CameraScanner.cpp

include/
└── scanner/
    └── CameraScanner.hpp
```

### Shared Technologies

| Technology | Purpose |
|---|---|
| C++20 | Programming language |
| CMake | Build system |
| SFML | Graphical interface |
| ZXing-C++ | Barcode generation and decoding |
| OpenCV | Camera capture and frame processing |

### Collaboration Guidelines

- Each member works on their assigned module and branch.
- Follow the shared project structure and coding conventions.
- Coordinate function names and interfaces before integration.
- Push changes to individual branches.
- Submit Pull Requests to `main`.
- The project lead reviews and integrates contributions.

###
