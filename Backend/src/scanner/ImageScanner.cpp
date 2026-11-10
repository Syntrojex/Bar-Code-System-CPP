#include "scanner/ImageScanner.h"

#include <QElapsedTimer>
#include <QPainter>

#ifdef HAVE_OPENCV
#include <opencv2/imgproc.hpp>
#include "Decoder.h"
#include "Scanner.h"

namespace {
// QImage -> BGR cv::Mat (transparent pixels are flattened onto white first).
cv::Mat toBgr(const QImage &src)
{
    QImage flat = src;
    if (src.hasAlphaChannel()) {
        flat = QImage(src.size(), QImage::Format_RGB32);
        flat.fill(Qt::white);
        QPainter p(&flat);
        p.drawImage(0, 0, src);
    }
    const QImage rgb = flat.convertToFormat(QImage::Format_RGB888);
    const cv::Mat view(rgb.height(), rgb.width(), CV_8UC3, const_cast<uchar *>(rgb.constBits()),
                       static_cast<size_t>(rgb.bytesPerLine()));
    cv::Mat bgr;
    cv::cvtColor(view, bgr, cv::COLOR_RGB2BGR);
    return bgr;
}
} // namespace
#endif

ImageScanResult ImageScanner::scan(const QImage &image) const
{
    ImageScanResult res;
    if (image.isNull()) {
        res.success = false;
        res.error = "Invalid or empty image provided.";
        return res;
    }

#ifndef HAVE_OPENCV
    res.error = "This build has no OpenCV, so barcode images cannot be decoded. Install OpenCV and reconfigure CMake.";
    return res;
#else
    QElapsedTimer timer;
    timer.start();

    const cv::Mat original = toBgr(image);
    Scanner scanner;                                // only its frame decoder is used (no camera is opened)
    Decoder decoder;

    // try the image as it is, then scaled to a size the barcode finder handles well
    std::vector<double> scales{1.0};
    const int longest = std::max(original.cols, original.rows);
    if (longest > 1400) scales.push_back(1000.0 / longest);
    if (longest < 500) scales.push_back(2.0);

    bool badCheck = false;
    for (double s : scales) {
        cv::Mat bgr;
        if (s == 1.0) bgr = original;
        else cv::resize(original, bgr, cv::Size(), s, s, s < 1.0 ? cv::INTER_AREA : cv::INTER_CUBIC);

        std::string six;
        cv::Rect box;
        if (scanner.frameRead(six, box, bgr)) {
            res.success = true;
            res.text = QString::fromStdString(six);
            if (box.area() > 0)
                res.box = QRect(qRound(box.x / s), qRound(box.y / s), qRound(box.width / s), qRound(box.height / s));
            res.latencyMs = static_cast<int>(timer.elapsed());
            return res;
        }

        // the image may be just the barcode itself (clean export): decode the whole picture
        cv::Mat gray, padded;
        cv::cvtColor(bgr, gray, cv::COLOR_BGR2GRAY);
        cv::copyMakeBorder(gray, padded, 40, 40, 60, 60, cv::BORDER_CONSTANT, cv::Scalar(255));
        if (decoder.decodeCropping(padded, six)) {
            res.success = true;
            res.text = QString::fromStdString(six);
            res.box = QRect(0, 0, image.width(), image.height());
            res.latencyMs = static_cast<int>(timer.elapsed());
            return res;
        }
        badCheck = badCheck || decoder.isBadCheck();
    }

    res.latencyMs = static_cast<int>(timer.elapsed());
    res.error = badCheck ? "Check digit mismatch: the barcode is damaged or is not a valid EAN-8."
                         : "No readable EAN-8 barcode found in this image.";
    return res;
#endif
}
