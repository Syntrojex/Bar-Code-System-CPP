# Increment 1 — Barcode Generator and Reader

---

## Overview

The first increment focuses on developing a working version of the C++ desktop application with barcode generation and reading capabilities.

The application must be able to generate a barcode from a six-digit number, display it on the screen, save it as an image, and read the generated barcode through either an uploaded image or a laptop camera.

---

## Requirements

### 1. Barcode Generation

The application must:

- Accept a six-digit numeric input from the user.
- Ensure that the first digit is non-zero.
- Generate a barcode corresponding to the provided number.
- Display the generated barcode on the application screen.
- Save the generated barcode to disk as an image.

### 2. Barcode Reading

The application must be able to read a barcode generated through the application.

**A. Image Upload**

- Allow the user to select and upload a barcode image.
- Read and decode the barcode from the selected image.
- Display the decoded six-digit number on the screen.

**B. Laptop Camera**

- Allow the user to scan a barcode using the laptop camera.
- Detect and decode the barcode from the camera feed.
- Display the decoded six-digit number on the screen.

---

## Technical Constraints

- Existing non-standard libraries must not be used for barcode generation.
- Libraries for image processing, file handling, and other supporting operations are permitted.
- The application must be developed in C++.
- All submissions must be made through GitHub.

---

## Team Contributions

### Farzam — UI Development & Barcode Reading

- UI Development (with Ali)
- Barcode Reading (with Shauzab)
- Barcode Decoding
- Decoded Value Return Handling
- Invalid Input Handling
- File Handling (with Mustafa)

### Ali — UI Development & Barcode Generation

- UI Development (with Farzam)
- Barcode Generation
- Image Processing
- Input Handling
- Taking 6-Digit Number as Input

### Mustafa — Project Lead, Management & Integration

- Project Management
- Project Integration
- Error Handling
- File Handling (with Farzam)
- CMake Build Configuration
- Live Scanner Implementation (with Farzam and Shauzab)

### Shauzab — Camera & Live Scanner Development

- Barcode Reading (with Farzam)
- Live Scanner Implementation
- Camera Opening and Initialization
- Camera Frame Capturing
- Scanner Functionality

---

## Notes

- The generated barcode must represent the original six-digit input.
- Both image upload and laptop camera scanning are required reading methods.
- Further details about the products, files, and their formats will be provided after Increment 1.
