#pragma once

#include <QImage>
#include <QRect>
#include <QString>

struct ImageScanResult
{
    bool success = false;
    QString text;          // the decoded 6-digit code
    QString error;
    QRect box;             // where the barcode was found (image coordinates, empty if unknown)
    int latencyMs = 0;     // time spent decoding
};

class ImageScanner
{
public:
    // Farzam: uploaded QImage se barcode decode karega (EAN-8 via Backend/Decoder, needs OpenCV).
    ImageScanResult scan(const QImage &image) const;
};
