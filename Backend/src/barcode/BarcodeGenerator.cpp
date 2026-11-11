#include "barcode/BarcodeGenerator.h"
#include <QFont>
#include <QPainter>
#include <QRegularExpression>

namespace {
// EAN-8 left-hand ("L") digit patterns; right-hand digits use the bitwise complement.
const char *const kLeft[10] = {"0001101", "0011001", "0010011", "0111101", "0100011",
                               "0110001", "0101111", "0111011", "0110111", "0001011"};

bool validSix(const QString &s)
{
    static const QRegularExpression re("^[1-9][0-9]{5}$");
    return re.match(s).hasMatch();
}

QString modulesFor(const QString &eight)
{
    QString bits = "101";
    for (int i = 0; i < 4; ++i)
        bits += QLatin1String(kLeft[eight.at(i).digitValue()]);
    bits += "01010";
    for (int i = 4; i < 8; ++i) {
        QString p = QLatin1String(kLeft[eight.at(i).digitValue()]);
        for (QChar &c : p) c = (c == QLatin1Char('1')) ? QLatin1Char('0') : QLatin1Char('1');
        bits += p;
    }
    bits += "101";
    return bits;                                    // 3 + 28 + 5 + 28 + 3 = 67 modules
}
} // namespace

QString BarcodeGenerator::ean8Payload(const QString &six)
{
    if (!validSix(six)) return QString();
    const QString d = "0" + six;                    // 7 digits
    int sum = 0;
    for (int i = 0; i < 7; ++i)
        sum += d.at(i).digitValue() * ((i % 2 == 0) ? 3 : 1);
    return d + QChar('0' + (10 - sum % 10) % 10);
}

BarcodeResult BarcodeGenerator::generate(const QString &number, bool humanReadable) const
{
    BarcodeResult res;
    if (!validSix(number)) {
        res.success = false;
        res.error = "Input must be exactly 6 digits, with the first digit non-zero.";
        return res;
    }

    const QString eight = ean8Payload(number);
    const QString bits = modulesFor(eight);

    // High-resolution, print-friendly raster output (approximately 300 DPI at 8 inches).
    // Integer module widths keep every bar edge crisp and scanner-friendly.
    const int w = 2400;
    const int h = 960;
    const int module = 24;                          // 67 modules = 1608 px; generous quiet zones remain on both sides
    const int x0 = (w - bits.size() * module) / 2;
    const int barTop = 104;
    const int barHeight = humanReadable ? 600 : 752;

    QImage img(w, h, QImage::Format_RGB32);
    img.setDotsPerMeterX(11811);                    // 300 DPI metadata
    img.setDotsPerMeterY(11811);
    img.fill(Qt::white);

    QPainter painter(&img);
    painter.setRenderHint(QPainter::Antialiasing, false);
    painter.setPen(Qt::NoPen);
    painter.setBrush(Qt::black);
    for (int i = 0; i < bits.size(); ++i) {
        if (bits.at(i) == QLatin1Char('1'))
            painter.drawRect(x0 + i * module, barTop, module, barHeight);
    }

    if (humanReadable) {
        painter.setRenderHint(QPainter::TextAntialiasing, true);
        QFont font("Consolas");
        font.setStyleHint(QFont::Monospace);
        font.setPixelSize(112);
        font.setBold(true);
        painter.setFont(font);
        painter.setPen(Qt::black);
        const int textTop = barTop + barHeight + 32;
        for (int i = 0; i < 8; ++i) {
            const int cell = (i < 4) ? (3 + 7 * i) : (36 + 7 * (i - 4));   // module index of this digit's 7-module cell
            painter.drawText(QRect(x0 + cell * module, textTop, 7 * module, 144), Qt::AlignCenter, QString(eight.at(i)));
        }
    }
    painter.end();

    res.success = true;
    res.image = img;
    return res;
}
