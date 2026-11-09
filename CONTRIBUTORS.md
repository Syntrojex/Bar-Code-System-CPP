# Contributors

**Bar-Code-System-CPP** is a collaborative academic project developed as part of the Introduction to Software Engineering course.

## Team Members

| Name | Roll Number | Role | GitHub Profile |
|---|---|---|---|
| Muhammad Mustafa Amir | 25L-3107 | Project Lead & Developer | [@Syntrojex](https://github.com/Syntrojex) |
| Farzam Zeeshan | 25L-3049 | Developer | [@farzamzeeshan1](https://github.com/farzamzeeshan1) |
| Ali Abdullah | 25L-3022 | Developer | [@aliabdullah-se](https://github.com/aliabdullah-se) |
| Shauzab Mukhtar | 25L-3084 | Developer | [@shauzab-72](https://github.com/shauzab-72) |

---

**Project:** Barcode System

**Course:** Introduction to Software Engineering

**Institution:** FAST-NUCES

**Development Language:** C++


## Ali — Barcode Generation Module: Feature & Verification Log

Project: BarcodeHub / Bar-Code-System-CPP
Branch: ali (local; remote divergence not yet reconciled)
Last verification date: 2026-11-09
Status: Generator implemented and integrated with UI; compiled, displayed, and exported. External scanner interoperability not yet verified.
GEN-001 — Six-digit numeric input validation
- What: Accepts exactly six numeric ASCII digits with first digit 1–9.
- Why: Increment 1 contract requires six-digit payload and excludes leading zero.
- Implementation: QRegularExpression("^[1-9][0-9]{5}$") inside BarcodeGenerator::generate; GeneratePage validates UI entry and disables Generate on invalid input.
- API: BarcodeResult BarcodeGenerator::generate(const QString &number) const returns failure with descriptive error for invalid input.
- Testing: Source inspected and app compiled; invalid-input GUI negative test still pending.
GEN-002 — Manual Code 128 Code Set C encoding
- What: Converts six input digits into three digit pairs and encodes a valid Code 128-C symbol sequence.
- Why: Must produce decodable bars without calling a barcode-generation library.
- Algorithm: Start C (105); three numeric pair symbols; modulo-103 weighted checksum; Stop (106). Example 123456: symbols 105, 12, 34, 56, 44, 106, checksum (105 + 12 + 2*34 + 3*56) % 103 = 44.
- Rendering: Handwritten 107-symbol width-pattern table, alternating bar/space runs painted with Qt QPainter into black-on-white QImage; quiet zones included; digits underneath.
- Code: Backend/src/barcode/BarcodeGenerator.cpp; header Backend/include/barcode/BarcodeGenerator.h unchanged.
- Build evidence: User's cmake --build --preset local-opencv completed and linked BarcodeHub.exe without compilation errors on 2026-11-09.
- Image evidence: User uploaded 123456.png (900×360). Pixel-run inspection of barcode region yielded 37 alternating runs matching Code 128-C symbols 105,12,34,56,44,106 after scaling; checksum verified as 44.
- Limitations: Pixel-rendered export has undergone UI scaling/interpolation; actual interoperability with a separately implemented barcode decoder/hardware scanner remains untested.
GEN-003 — Generator → UI integration and PNG export
- What: GeneratePage::generateRequested is wired in MainWindow to BarcodeGenerator::generate; successful result supplied with GeneratePage::setBarcodeImage(QPixmap::fromImage(result.image)).
- Why: Replace the UI-only mock barcode preview with actual encoded output.
- Code: Frontend/src/mainwindow.cpp signal/slot lambda, existing GeneratePage::onSave and BarcodeView::renderBarcode export path.
- Evidence: User launched BarcodeHub successfully after adding Qt/MinGW DLL directories to terminal PATH, generated and viewed barcode for 123456, saved 123456.png, and uploaded image for inspection.
- Limitations / follow-up: renderBarcode() scales the preview to 900×360 rather than saving the generator's original QImage directly. Improve export to preserve sharp module boundaries, then test independent decoding. Error display path for result.success == false could be improved.
Pending verification
- Test edge-valid payloads (100000, 999999, digits containing 00), leading zero, short/long/non-numeric inputs.
- Independent decode of exported barcode using teammate's scanner or trusted reference implementation.
- Preserve original pixel module widths in PNG export (avoid smooth rescaling); check scanner interoperability.
- Reconcile diverged local/remote ali branch safely before commit/push/PR; preserve teammates' changes.