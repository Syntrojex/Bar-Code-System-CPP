#pragma once

#include <QImage>
#include <QString>

struct BarcodeResult
{
    bool success = false;
    QImage image;
    QString error;
};

class BarcodeGenerator
{
public:
    // Ali: six-digit input validate karke barcode image generate karega.
    BarcodeResult generate(const QString &number) const;
};
