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

Two input methods must be supported:

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

## Team Responsibilities

The work for this increment is divided among the four team members as follows:

### 1. Mustafa — Project Lead & Barcode Generation

- Implement barcode generation functionality.
- Handle six-digit input validation.
- Generate and save barcode images.
- Manage project integration and overall code coordination.
- Handle error management and testing.
- Review and merge team contributions.

### 2. Ali — Frontend Development

- Develop the desktop application interface using Qt.
- Design the barcode generation interface.
- Design the barcode image upload interface.
- Design the camera scanning interface.
- Display generated barcodes and decoded results.
- Connect frontend components with backend functionality.

### 3. Farzam — Image-Based Barcode Reading

- Implement barcode reading from uploaded images.
- Handle image input and processing.
- Integrate barcode decoding functionality.
- Return the decoded number to the application.

### 4. Shozab — Camera-Based Barcode Reading

- Implement camera capture using OpenCV.
- Process live camera frames.
- Detect and read barcodes from the camera feed.
- Integrate barcode decoding functionality.
- Handle camera-related operations.

---

## Acceptance Criteria

The increment must satisfy the following requirements:

- [ ] The application accepts valid six-digit numbers.
- [ ] Inputs beginning with zero are rejected.
- [ ] A barcode is successfully generated for a valid input.
- [ ] The generated barcode is displayed on the screen.
- [ ] The generated barcode is saved as an image on disk.
- [ ] The application successfully reads a generated barcode from an uploaded image.
- [ ] The application successfully reads a generated barcode through the laptop camera.
- [ ] The decoded six-digit number is displayed correctly on the screen.

---

## Submission & Evaluation

| Detail | Information |
|---|---|
| Submission Platform | GitHub |
| Evaluation | Graded In-Class Demonstration |
| Demonstration Date | November 12, 2026 |

---

## Notes

- The generated barcode must represent the original six-digit input.
- Both image upload and laptop camera scanning are required reading methods.
- Further details about the products, files, and their formats will be provided after Increment 1.