#pragma once

#include <QImage>
#include <QString>

struct BarcodeResult
{
    bool success = false;
    QImage image;
    QString error;
};

// Generates an EAN-8 barcode (the symbology Backend/Decoder reads).
// The 6-digit input is encoded as: "0" + <6 digits> + <mod-10 check digit>.
class BarcodeGenerator
{
public:
    // Validates the six-digit input (first digit non-zero) and renders a 600x240 EAN-8 image.
    // humanReadable = print the 8 digits under the bars.
    BarcodeResult generate(const QString &number, bool humanReadable = true) const;

    // The 8-digit EAN-8 payload for a valid 6-digit code ("" if the code is invalid).
    static QString ean8Payload(const QString &six);
};
