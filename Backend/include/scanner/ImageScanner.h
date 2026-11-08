#pragma once

#include <QImage>
#include <QString>

struct ImageScanResult
{
    bool success = false;
    QString text;
    QString error;
};

class ImageScanner
{
public:
    // Farzam: uploaded QImage se barcode decode karega.
    ImageScanResult scan(const QImage &image) const;
};
