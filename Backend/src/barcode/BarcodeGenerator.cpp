#include "barcode/BarcodeGenerator.h"
#include <QRegularExpression>
#include <QPainter>
#include <QFont>

BarcodeResult BarcodeGenerator::generate(const QString &number) const
{
    BarcodeResult res;
    static const QRegularExpression re("^[1-9][0-9]{5}$");
    if (!re.match(number).hasMatch()) {
        res.success = false;
        res.error = "Input must be exactly 6 digits, with the first digit non-zero.";
        return res;
    }

    const int w = 450;
    const int h = 180;
    QImage img(w, h, QImage::Format_ARGB32_Premultiplied);
    img.fill(Qt::white);

    QPainter painter(&img);
    painter.setRenderHint(QPainter::Antialiasing, false);
    painter.setRenderHint(QPainter::TextAntialiasing, true);

    const int barTop = 20;
    const int barHeight = 110;
    int x = 35;

    // Start pattern
    painter.fillRect(x, barTop, 3, barHeight, Qt::black);
    x += 5;
    painter.fillRect(x, barTop, 2, barHeight, Qt::black);
    x += 6;

    // Digits bar representation
    for (const QChar &ch : number) {
        int d = ch.digitValue();
        int w1 = 2 + (d % 3);
        int sp1 = 2 + ((d + 1) % 2);
        int w2 = 1 + ((d + 2) % 3);
        int sp2 = 3 + (d % 2);

        painter.fillRect(x, barTop, w1, barHeight, Qt::black);
        x += w1 + sp1;
        painter.fillRect(x, barTop, w2, barHeight, Qt::black);
        x += w2 + sp2;
    }

    // Stop pattern
    painter.fillRect(x, barTop, 4, barHeight, Qt::black);
    x += 7;
    painter.fillRect(x, barTop, 2, barHeight, Qt::black);

    // Number label at bottom
    QFont font("Consolas", 16, QFont::Bold);
    painter.setFont(font);
    painter.setPen(Qt::black);

    QString spaced;
    for (int i = 0; i < number.size(); ++i) {
        if (i > 0) spaced += " ";
        spaced += number.at(i);
    }
    painter.drawText(QRect(0, barTop + barHeight + 8, w, 32), Qt::AlignCenter, spaced);

    res.success = true;
    res.image = img;
    return res;
}
