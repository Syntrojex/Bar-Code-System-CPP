#include "barcode/BarcodeGenerator.h"

#include <QFont>
#include <QPainter>
#include <QRegularExpression>
#include <QString>

namespace
{

    // EAN-8 digit patterns: each digit occupies exactly seven modules.
    // Left four digits use the L (A) set; right four use the R (C) set.
    constexpr const char *kLeftPatterns[10] = {
        "0001101", "0011001", "0010011", "0111101", "0100011",
        "0110001", "0101111", "0111011", "0110111", "0001011"};

    constexpr const char *kRightPatterns[10] = {
        "1110010", "1100110", "1101100", "1000010", "1011100",
        "1001110", "1010000", "1000100", "1001000", "1110100"};

    int ean8CheckDigit(const QString &sevenDigits)
    {
        int weightedSum = 0;
        for (int i = 0; i < sevenDigits.size(); ++i)
        {
            const int digit = sevenDigits.at(i).digitValue();
            weightedSum += digit * ((i % 2 == 0) ? 3 : 1);
        }
        return (10 - (weightedSum % 10)) % 10;
    }

    QString ean8Modules(const QString &eightDigits)
    {
        QString bits;
        bits.reserve(67);
        bits += QStringLiteral("101"); // Left guard

        for (int i = 0; i < 4; ++i)
            bits += QString::fromLatin1(kLeftPatterns[eightDigits.at(i).digitValue()]);

        bits += QStringLiteral("01010"); // Centre guard

        for (int i = 4; i < 8; ++i)
            bits += QString::fromLatin1(kRightPatterns[eightDigits.at(i).digitValue()]);

        bits += QStringLiteral("101"); // Right guard
        return bits;
    }

} // namespace

BarcodeResult BarcodeGenerator::generate(const QString &number) const
{
    BarcodeResult result;

    // Preserve the project's six-digit, nonzero-leading-digit input requirement.
    static const QRegularExpression validCode(QStringLiteral("^[1-9][0-9]{5}$"));
    if (!validCode.match(number).hasMatch())
    {
        result.error = QStringLiteral("Input must be exactly 6 digits, with the first digit non-zero.");
        return result;
    }

    // EAN-8 encodes seven data digits + one check digit. We add a zero to the
    // six-digit project payload, then calculate the official modulo-10 check.
    // Example: 123456 -> 0123456 -> check 5 -> EAN-8 01234565.
    const QString sevenDigits = QStringLiteral("0") + number;
    const QString fullCode = sevenDigits + QString::number(ean8CheckDigit(sevenDigits));
    const QString bits = ean8Modules(fullCode); // Exactly 67 narrow modules.

    constexpr int kModulePx = 4;
    constexpr int kQuietModules = 10; // Generous white quiet zones on both sides.
    constexpr int kBarTop = 14;
    constexpr int kBarHeight = 124;
    constexpr int kGuardExtra = 10;
    constexpr int kImageHeight = 190;
    constexpr int kTotalModules = 67 + 2 * kQuietModules;

    QImage image(kTotalModules * kModulePx, kImageHeight, QImage::Format_RGB32);
    if (image.isNull())
    {
        result.error = QStringLiteral("Could not allocate barcode image.");
        return result;
    }
    image.fill(Qt::white);

    QPainter painter(&image);
    painter.setRenderHint(QPainter::Antialiasing, false);
    painter.setPen(Qt::NoPen);
    painter.setBrush(Qt::black);

    for (int i = 0; i < bits.size(); ++i)
    {
        if (bits.at(i) != QLatin1Char('1'))
            continue;

        // EAN-8 guard bars visually extend below the data bars.
        const bool isGuard = (i < 3) || (i >= 31 && i < 36) || (i >= 64);
        const int height = kBarHeight + (isGuard ? kGuardExtra : 0);
        painter.fillRect((kQuietModules + i) * kModulePx,
                         kBarTop, kModulePx, height, Qt::black);
    }

    painter.setPen(Qt::black);
    QFont font(QStringLiteral("Consolas"), 16, QFont::Bold);
    painter.setFont(font);
    painter.drawText(QRect(0, 151, image.width(), 30),
                     Qt::AlignCenter, fullCode);
    painter.end();

    result.success = true;
    result.image = image;
    return result;
}
