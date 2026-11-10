#include "barcode/BarcodeGenerator.h"
#include "barcode/Decoder.h"
#include "scanner/ImageScanner.h"
#include "scanner/CameraScanner.h"
#include "scanner/Scanner.h"
#include "mainwindow.h"
#include "generatepage.h"
#include "decodepage.h"
#include "widgets.h"
#include <QApplication>
#include <QElapsedTimer>
#include <QImageReader>
#include <QPainter>
#include <QTableWidget>
#include <QTemporaryDir>
#include <QThread>
#include <QTransform>
#include <iostream>
#include <functional>

static int failures = 0;
static void check(bool ok, const QString &name) {
    std::cout << (ok ? "PASS " : "FAIL ") << name.toStdString() << std::endl;
    if (!ok) ++failures;
}

static void roundTrip(const QImage &image, const QString &code, const QString &name) {
    const auto r = ImageScanner().scan(image);
    check(r.success && r.text == code, name + (r.success ? "" : ": " + r.error));
}

static bool waitFor(const std::function<bool()> &condition, int timeoutMs = 10000) {
    QElapsedTimer timer;
    timer.start();
    while (!condition() && timer.elapsed() < timeoutMs) {
        QApplication::processEvents();
        QThread::msleep(10);
    }
    return condition();
}

static int cameraSmoke() {
    const auto names = CameraScanner::availableCameras();
    check(!names.isEmpty(), "camera enumeration");
    for (const auto &name : names) std::cout << "DEVICE " << name.toStdString() << std::endl;
    if (names.isEmpty()) return 1;
    CameraScanner camera;
    for (int cycle = 0; cycle < 2; ++cycle) {
        QString error;
        const bool opened = camera.openCamera(0, &error);
        check(opened, "open camera cycle " + QString::number(cycle) + ": " + error);
        if (!opened) continue;
        int frames = 0;
        for (int i = 0; i < 20; ++i) {
            const auto frame = camera.readFrame();
            if (frame.success && !frame.frame.isNull()) ++frames;
        }
        check(frames == 20, "20 real frames with decoder enabled");
        camera.closeCamera();
        check(!camera.isOpened(), "camera released");
    }
    {
        MainWindow window;
        auto *decode = window.decodePage();
        decode->setCameraActive(true);
        decode->cameraStartRequested(0);
        check(waitFor([&] { return !decode->cameraView()->frame().isNull(); }), "threaded UI receives real camera frames");
        decode->cameraView()->setFrozen(true);
        const auto frozen = decode->cameraView()->frame().toImage();
        QElapsedTimer delay; delay.start();
        waitFor([&] { return delay.elapsed() > 400; }, 1000);
        check(frozen == decode->cameraView()->frame().toImage(), "live freeze preserves frame");
        decode->stopCamera();
        check(!decode->cameraActive() && decode->cameraView()->frame().isNull(), "UI stop clears camera state");
        // Stop immediately while the worker is opening: late callbacks must not restart it.
        decode->setCameraActive(true);
        decode->cameraStartRequested(0);
        decode->stopCamera();
        delay.restart();
        waitFor([&] { return delay.elapsed() > 1500; }, 2500);
        check(!decode->cameraActive() && decode->cameraView()->frame().isNull(), "cancel opening ignores stale callbacks");
    }
    QString error;
    check(camera.openCamera(0, &error), "window shutdown releases camera for reuse");
    camera.closeCamera();
    return failures ? 1 : 0;
}

int main(int argc, char **argv) {
    QApplication app(argc, argv);
    if (app.arguments().contains("--camera-smoke")) return cameraSmoke();
    BarcodeGenerator generator;
    for (const QString &bad : {QString(), QString("012345"), QString("12345"), QString("1234567"), QString("12a456")})
        check(!generator.generate(bad).success, "reject invalid input: " + bad);
    check(BarcodeGenerator::ean8Payload("123456") == "01234565", "known EAN-8 check digit");

    for (const QString &code : {QString("100000"), QString("123456"), QString("749205"), QString("999999")}) {
        for (bool digits : {false, true}) {
            const auto result = generator.generate(code, digits);
            check(result.success, "generate " + code);
            roundTrip(result.image, code, "round trip " + code + (digits ? " with text" : " without text"));
        }
    }
    const QImage original = generator.generate("749205").image;
    roundTrip(original.mirrored(true, false), "749205", "mirrored barcode");
    roundTrip(original.transformed(QTransform().rotate(90)), "749205", "vertical barcode");
    roundTrip(original.transformed(QTransform().rotate(180)), "749205", "upside down barcode");
    roundTrip(original.scaled(300, 120, Qt::IgnoreAspectRatio, Qt::SmoothTransformation), "749205", "small barcode");
    roundTrip(original.scaled(1800, 720), "749205", "large barcode");
    QImage tilted(900, 500, QImage::Format_RGB32);
    tilted.fill(Qt::white);
    { QPainter p(&tilted); p.translate(450, 250); p.rotate(12); p.drawImage(-300, -120, original); }
    roundTrip(tilted, "749205", "12 degree tilted barcode");
    QImage transparent = original.convertToFormat(QImage::Format_ARGB32);
    for (int y = 0; y < transparent.height(); ++y)
        for (int x = 0; x < transparent.width(); ++x)
            if (qGray(transparent.pixel(x, y)) > 245) transparent.setPixel(x, y, qRgba(0, 0, 0, 0));
    roundTrip(transparent, "749205", "transparent background");
    QTemporaryDir dir;
    const QString path = dir.filePath(QString::fromUtf8("barcode_\xE6\xB5\x8B\xE8\xAF\x95.png"));
    check(original.save(path), "save PNG at Unicode path");
    roundTrip(QImage(path), "749205", "reload saved PNG");
    const QString jpeg = dir.filePath("barcode.jpg");
    check(original.save(jpeg, "JPG", 75), "save JPEG");
    roundTrip(QImage(jpeg), "749205", "JPEG decode");
    QImage blank(600, 240, QImage::Format_RGB32); blank.fill(Qt::white);
    check(!ImageScanner().scan(blank).success, "reject blank image");
    check(!ImageScanner().scan(QImage()).success, "reject empty image");
    QImage damaged = original;
    { QPainter p(&damaged); p.fillRect(250, 0, 30, damaged.height(), Qt::white); }
    check(!ImageScanner().scan(damaged).success, "reject damaged bars");
    // Replace only the check digit with another valid pattern: structure still passes.
    QImage wrongCheck = generator.generate("123456", false).image;
    { QPainter p(&wrongCheck); p.fillRect(464, 26, 49, 188, Qt::white);
      const QString bits = "1110010"; // right-hand encoding of zero, instead of five
      for (int i = 0; i < 7; ++i) if (bits[i] == '1') p.fillRect(464 + 7*i, 26, 7, 188, Qt::black); }
    check(!ImageScanner().scan(wrongCheck).success, "reject wrong checksum");
    std::string stale = "123456";
    Decoder decoder;
    check(!decoder.decodeSix("bad", stale) && stale.empty(), "failed decode clears previous output");

    // Exercise the actual frontend-to-backend signal connections.
    {
        MainWindow window;
        auto *decode = window.decodePage();
        auto *history = decode->findChild<QTableWidget *>("scanHistory");
        check(history && history->rowCount() == 0, "history starts empty");
        window.generatePage()->setCode("749205");
        auto *view = window.generatePage()->findChild<BarcodeView *>();
        roundTrip(view->renderBarcode().toImage(), "749205", "UI generated image decodes");
        decode->sampleRequested("749205", false);
        check(history->rowCount() == 1 && history->item(0, 2)->text() == "749205", "sample uses real decoder and logs result");
        decode->sampleRequested("749205", true);
        check(history->rowCount() == 1, "failed sample does not add success history");
        decode->setTab(2);
        for (auto *tabs : decode->findChildren<SegBar *>()) check(tabs->current() == 2, "tabs stay synchronized");
        decode->setCameraList({});
        check(!decode->cameraActive(), "no camera is inactive");
        CameraView camera;
        camera.setRunning(true);
        camera.setFrame(QPixmap::fromImage(original));
        camera.setFrozen(true);
        camera.setFrame(QPixmap::fromImage(blank));
        check(camera.frame().toImage() == original, "freeze retains displayed frame");
        camera.setRunning(false);
        check(camera.frame().isNull() && !camera.frozen(), "stop clears frame and frozen state");
    }
    std::cout << "Failures: " << failures << std::endl;
    return failures ? 1 : 0;
}
