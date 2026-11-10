# Bar-Code-System-CPP

## Run on Windows

Double-click **`Run-BarcodeHub.cmd`**, or launch
**`build/portable/BarcodeHub/BarcodeHub.exe`** after building.
The portable directory includes Qt plugins, OpenCV and MinGW runtime DLLs.
Copy the **whole directory**, not just the executable, to another Windows x64 PC.

### Build and install dependencies

Install the **Qt 6 MinGW 64-bit kit**, its matching MinGW compiler, CMake and Ninja
through the Qt installer. Then run from this repository in PowerShell:

```powershell
powershell -ExecutionPolicy Bypass -File scripts/build-windows.ps1
```

The script detects Qt under `C:\Qt` or `D:\Qt`, chooses its matching compiler,
downloads OpenCV **4.13.0** from the official OpenCV GitHub tag if sources are
missing, builds and installs it locally under `build/deps/opencv`, builds the
application, runs regression tests and creates the portable directory.
It does not require administrator access or change the system PATH.
The first OpenCV build takes several minutes. Later builds reuse it.

For another Qt installation, pass explicit paths:

```powershell
.\scripts\build-windows.ps1 -QtRoot C:\Qt\6.11.0\mingw_64 -MinGWRoot C:\Qt\Tools\mingw1310_64
```

Do not mix MSVC OpenCV binaries with a MinGW Qt kit. OpenCV is required: a build
without working image/camera support now fails at configuration time.
The setup script applies a narrow OpenCV 4.13 MinGW compatibility fix, replacing
the non-standard `M_PI` constant with OpenCV's `CV_PI` in `shapedescr.cpp`.

### Camera and scanning

- Open **Decode Barcode → Scan with Camera**, select a detected camera and
  requested resolution, then click **Start Camera**. The actual frame size is
  shown in the preview; cameras may negotiate a different resolution.
- **Freeze / Resume** freezes both the preview and the decoded result.
  **Mirror** changes only the preview. **Snapshot** saves the displayed frame.
- **Autofocus** requests hardware autofocus where supported. Fixed-focus
  webcams need suitable distance, sharp bars and good lighting.
- Connect a camera and use the refresh button if the device list is empty.
  Stop the camera before changing devices or resolution. Leaving the camera
  tab or closing the application releases it.
- Successful scans appear in **Scan Logs**. CSV export and clear history work;
  logs are held for this session (up to 1,000 entries). Automatic clipboard copy
  is optional and starts off.
- Upload a PNG/JPEG/BMP image or use the built-in clean/damaged samples to
  check decoding without a camera.

This increment supports **the project's EAN-8 format**: a nonzero-leading
six-digit code encoded as `0 + code + Mod-10 check digit`. It does not claim QR,
Code 128, general retail EAN-13, or arbitrary EAN-8 support. SVG export and
thermal printing remain disabled. Printed-camera decoding depends on focus,
barcode size, lighting and camera quality.

If opening fails, close other camera apps and check Windows **Settings →
Privacy & security → Camera → Let desktop apps access your camera**. Reconnect
the webcam and refresh. This application uses OpenCV's DirectShow backend on
Windows and processes frames locally.

### Verify

```powershell
ctest --test-dir build/local-release --output-on-failure
.\build\local-release\barcode_tests.exe --camera-smoke
```

Regression tests cover generation/decode round trips, saved PNG/JPEG images,
Unicode paths, rotation/mirroring, transparent backgrounds, malformed images,
checksum rejection and frontend integration. The separate camera smoke test
opens the first real camera twice, reads frames with decoding enabled and
checks release/reopen; it requires an attached camera.

See [OpenCV build options](https://docs.opencv.org/4.13.0/db/d05/tutorial_config_reference.html)
and [OpenCV source release](https://github.com/opencv/opencv/tree/4.13.0).

A C++ desktop application designed to simplify inventory management and sales operations for small retail stores, supermarkets, and corner shops.

---

## Project Overview

Bar-Code-System-CPP is an academic term project developed for the **Introduction to Software Engineering (ISE)** course at FAST-NUCES.

The application aims to provide a simple and efficient system through which store employees can manage inventory and record sales with minimal manual input.

Cashiers will be able to scan product barcodes, retrieve product information, add items to orders, and generate bills. Managers will also be able to generate barcodes for products in the store.

The project is being developed incrementally, with a focus on building a reliable, user-friendly, and maintainable desktop application.

---

## Key Features

- **Barcode Generation** — Generate barcodes for products.
- **Barcode Scanning** — Read barcodes using a laptop camera.
- **Image-Based Reading** — Upload barcode images for decoding.

---

## Technology Stack

- **C++** — Core application logic.
- **Qt** — Desktop graphical user interface.
- **CMake** — Build system and project configuration.
- **PowerShell setup script** — Reproducible local OpenCV build and Windows packaging.
- **OpenCV** — Image processing and camera capture.

---

## Project Structure

```text
Bar-Code-System-CPP/
│
├── Frontend/             # Desktop user interface
├── Backend/              # Core application logic
├── Project-Planning/     # Increment-wise project documentation
│
├── CMakeLists.txt        # Build configuration
├── CMakePresets.json     # CMake build presets
├── .gitignore
└── README.md
```

---

## Development Workflow

The project follows a collaborative GitHub workflow:

1. Each team member works on an assigned branch.
2. Changes are committed and pushed to GitHub.
3. A Pull Request is opened against the `main` branch.
4. Changes are reviewed before being merged into the main project.

---

## Academic Information

| Detail | Information |
|---|---|
| Project | Bar-Code-System-CPP |
| Course | Introduction to Software Engineering (ISE) |
| University | FAST-NUCES |
| Semester | 3rd Semester |
| Instructor | Professor Rana Zeeshan |
| Language | C++ |
| Submission Platform | GitHub |

---

**Bar-Code-System-CPP** — Building a practical retail management solution, one increment at a time.
