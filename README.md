# 🏷️ BarcodeHub

<p align="center">
  <img src="https://img.shields.io/badge/C%2B%2B-17-00599C?style=for-the-badge&logo=cplusplus&logoColor=white" />
  <img src="https://img.shields.io/badge/Qt-6-41CD52?style=for-the-badge&logo=qt&logoColor=white" />
  <img src="https://img.shields.io/badge/OpenCV-5C3EE8?style=for-the-badge&logo=opencv&logoColor=white" />
  <img src="https://img.shields.io/badge/CMake-Build%20System-064F8C?style=for-the-badge&logo=cmake&logoColor=white" />
  <img src="https://img.shields.io/badge/Platform-Windows%20x64-0078D4?style=for-the-badge&logo=windows&logoColor=white" />
</p>

<p align="center">
  **A C++ desktop application for barcode generation, image decoding, and real-time camera scanning.**
  <br />
  Developed as an academic project for Introduction to Software Engineering at FAST-NUCES.
</p>

---

## 📌 Project Overview

**BarcodeHub** is a collaborative academic project developed for the **Introduction to Software Engineering (ISE)** course at FAST-NUCES.

The application aims to simplify product identification and barcode-related operations in retail environments through a desktop interface.

Developed incrementally, the project focuses on reliability, usability, and maintainable software architecture.

---

## ✨ Key Features

- 🏷️ **Barcode Generation** — Generate barcodes from supported six-digit product codes.
- 📷 **Camera Scanning** — Scan barcodes using connected cameras.
- 🖼️ **Image Decoding** — Decode barcodes from uploaded images.
- 📋 **Scan History** — Maintain session-based scan logs.
- 📤 **CSV Export** — Export scan history.
- 🎨 **Desktop GUI** — Interactive interface built with Qt Widgets.

---

## 📷 Camera & Image Scanning

- Select a camera and resolution to start live scanning.
- Freeze, resume, mirror, and capture camera frames.
- Upload PNG, JPEG, or BMP images for decoding.
- View successful scans in Scan Logs and export them as CSV.

---

## 🔢 Supported Barcode Format

The current increment supports a project-specific six-digit barcode format:

- **Input:** Six digits, starting with a nonzero digit.
- **Encoding:** `0 + code + Mod-10 check digit`.
- **Decoding:** Format and checksum validation.

This is a project-specific encoding based on an EAN-8-sized structure. It does not claim support for arbitrary EAN-8, EAN-13, QR codes, or Code 128.

SVG export and thermal printing are currently disabled.

---

## 🛠️ Technology Stack

| Technology | Purpose |
|---|---|
| **C++17** | Core application logic |
| **Qt 6 Widgets** | Desktop graphical interface |
| **OpenCV** | Image processing and camera capture |
| **CMake** | Build configuration |
| **MinGW-w64** | Windows C++ compilation |
| **PowerShell** | Automated setup and packaging |
| **Git & GitHub** | Version control and collaboration |
| **GitHub Actions** | Automatic branch synchronization |

---

## 📂 Project Structure

```text
Bar-Code-System-CPP/
│
├── Backend/                  # Core application logic
│   ├── include/
│   └── src/
│
├── Frontend/                 # Qt desktop interface
│   ├── assets/
│   ├── include/
│   └── src/
│
├── Project-Planning/         # Increment-wise documentation
├── scripts/                  # Build and setup scripts
│
├── .github/
│   ├── CODEOWNERS
│   └── workflows/
│
├── CMakeLists.txt
├── CMakePresets.json
├── project.json
├── CHANGELOG.md
├── CONTRIBUTORS.md
├── CONTRIBUTING.md
├── LICENSE
└── README.md
```

---

## 🎓 Academic Information

| Detail | Information |
|---|---|
| Project | BarcodeHub |
| Repository | Bar-Code-System-CPP |
| Course | Introduction to Software Engineering (ISE) |
| Institution | FAST-NUCES |
| Semester | 3rd Semester |
| Instructor | Professor Rana Zeeshan |
| Language | C++ |
| Methodology | Incremental Development |
| Submission Platform | GitHub |

---

## 👥 Project Team & Credits

This project was collaboratively developed by the following team members:

| Team Member | Roll Number | GitHub Profile |
|---|---|---|
| **Muhammad Mustafa Amir** | 25L-3107 | [@Syntrojex](https://github.com/Syntrojex) |
| **Ali Abdullah** | 25L-3022 | [@aliabdullah-se](https://github.com/aliabdullah-se) |
| **Farzam Zeeshan** | 25L-3049 | [@farzamzeeshan1](https://github.com/farzamzeeshan1) |
| **Shauzab Mukhtar** | 25L-3084 | [@shauzab-72](https://github.com/shauzab-72) |

---

## 🤝 Contributing

Contributions, suggestions, and improvements are welcome.

Please read the [Contributing Guidelines](CONTRIBUTING.md) before making changes to the project.

---

<p align="center">
  <b>BarcodeHub</b>
  <br />
  Building a practical barcode solution, one increment at a time.
  <br /><br />
  <sub>Developed with ❤️ by the BarcodeHub team.</sub>
</p>
