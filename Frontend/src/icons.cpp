#include "icons.h"

#include <QPainter>
#include <QPainterPath>
#include <QPolygonF>

QPixmap iconPixmap(const QString &n, const QColor &c, int size)
{
    const qreal dpr = 2.0;
    QPixmap pm(int(size * dpr), int(size * dpr));
    pm.setDevicePixelRatio(dpr);
    pm.fill(Qt::transparent);

    QPainter p(&pm);
    p.setRenderHint(QPainter::Antialiasing);
    p.scale(size / 20.0, size / 20.0);
    QPen pen(c, 1.7, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    p.setPen(pen);
    p.setBrush(Qt::NoBrush);
    auto dot = [&](qreal x, qreal y, qreal r) {
        p.setPen(Qt::NoPen); p.setBrush(c); p.drawEllipse(QPointF(x, y), r, r);
        p.setPen(pen); p.setBrush(Qt::NoBrush);
    };

    if (n == "barcode") {
        p.setPen(Qt::NoPen); p.setBrush(c);
        const qreal xs[] = {2.5, 5, 7.2, 9.5, 12.4, 14.4, 16.6};
        const qreal ws[] = {1.4, 1.0, 1.8, 1.0, 1.4, 1.0, 1.0};
        for (int i = 0; i < 7; ++i) p.drawRect(QRectF(xs[i], 3.5, ws[i], 13));
    } else if (n == "scan") {
        p.drawLine(QPointF(3, 7.5), QPointF(3, 3)); p.drawLine(QPointF(3, 3), QPointF(7.5, 3));
        p.drawLine(QPointF(12.5, 3), QPointF(17, 3)); p.drawLine(QPointF(17, 3), QPointF(17, 7.5));
        p.drawLine(QPointF(3, 12.5), QPointF(3, 17)); p.drawLine(QPointF(3, 17), QPointF(7.5, 17));
        p.drawLine(QPointF(12.5, 17), QPointF(17, 17)); p.drawLine(QPointF(17, 17), QPointF(17, 12.5));
        p.drawLine(QPointF(6, 10), QPointF(14, 10));
    } else if (n == "camera") {
        p.drawRoundedRect(QRectF(2.5, 5.5, 10.5, 9), 2, 2);
        QPolygonF t; t << QPointF(13.5, 8.8) << QPointF(17.5, 6.3) << QPointF(17.5, 13.7) << QPointF(13.5, 11.2);
        p.drawPolygon(t);
    } else if (n == "upload" || n == "download") {
        const bool up = (n == "upload");
        const qreal a = up ? 3.5 : 12.5, b = up ? 12.5 : 3.5;
        p.drawLine(QPointF(10, a), QPointF(10, b + (up ? 0 : 0)));
        if (up) { p.drawLine(QPointF(6.5, 7), QPointF(10, 3.5)); p.drawLine(QPointF(13.5, 7), QPointF(10, 3.5)); }
        else    { p.drawLine(QPointF(6.5, 9), QPointF(10, 12.5)); p.drawLine(QPointF(13.5, 9), QPointF(10, 12.5)); }
        p.drawLine(QPointF(3.5, 13), QPointF(3.5, 16.5)); p.drawLine(QPointF(3.5, 16.5), QPointF(16.5, 16.5));
        p.drawLine(QPointF(16.5, 16.5), QPointF(16.5, 13));
    } else if (n == "copy") {
        p.drawRoundedRect(QRectF(7.5, 7.5, 9, 9), 2, 2);
        QPainterPath b; b.moveTo(4.5, 12.5); b.lineTo(4.5, 5.5); b.quadTo(4.5, 3.5, 6.5, 3.5); b.lineTo(12.5, 3.5);
        p.drawPath(b);
    } else if (n == "check") {
        p.drawEllipse(QRectF(2.5, 2.5, 15, 15));
        QPainterPath t; t.moveTo(6.4, 10.2); t.lineTo(9, 12.8); t.lineTo(13.7, 7.6); p.drawPath(t);
    } else if (n == "info") {
        p.drawEllipse(QRectF(2.5, 2.5, 15, 15));
        p.drawLine(QPointF(10, 9), QPointF(10, 13.8)); dot(10, 6.5, 0.9);
    } else if (n == "trash") {
        p.drawLine(QPointF(3.5, 6), QPointF(16.5, 6));
        QPainterPath h; h.moveTo(7.8, 6); h.lineTo(7.8, 3.8); h.lineTo(12.2, 3.8); h.lineTo(12.2, 6); p.drawPath(h);
        QPainterPath b; b.moveTo(5, 6); b.lineTo(5.9, 16.2); b.lineTo(14.1, 16.2); b.lineTo(15, 6); p.drawPath(b);
        p.drawLine(QPointF(8.5, 9), QPointF(8.5, 13.5)); p.drawLine(QPointF(11.5, 9), QPointF(11.5, 13.5));
    } else if (n == "shield") {
        QPainterPath s; s.moveTo(10, 2.5); s.lineTo(16.5, 5); s.lineTo(16.5, 10);
        s.cubicTo(16.5, 14, 13.5, 16.5, 10, 17.5); s.cubicTo(6.5, 16.5, 3.5, 14, 3.5, 10); s.lineTo(3.5, 5); s.closeSubpath();
        p.drawPath(s);
        QPainterPath t; t.moveTo(7, 10); t.lineTo(9.2, 12.2); t.lineTo(13, 8); p.drawPath(t);
    } else if (n == "user") {
        p.drawEllipse(QRectF(7, 3.5, 6, 6));
        QPainterPath b; b.moveTo(4, 17); b.cubicTo(4, 13.5, 7, 12, 10, 12); b.cubicTo(13, 12, 16, 13.5, 16, 17); p.drawPath(b);
    } else if (n == "history") {
        p.drawEllipse(QRectF(3, 3, 14, 14));
        QPainterPath h; h.moveTo(10, 6.2); h.lineTo(10, 10); h.lineTo(12.8, 11.8); p.drawPath(h);
    } else if (n == "print") {
        p.drawRect(QRectF(5.5, 3, 9, 4.5));
        p.drawRoundedRect(QRectF(3, 7.5, 14, 6), 1.5, 1.5);
        p.drawRect(QRectF(5.5, 11.5, 9, 5.5));
    } else if (n == "hash") {
        p.drawLine(QPointF(8, 3.5), QPointF(6.5, 16.5)); p.drawLine(QPointF(13.5, 3.5), QPointF(12, 16.5));
        p.drawLine(QPointF(4, 7.5), QPointF(16.5, 7.5)); p.drawLine(QPointF(3.5, 12.5), QPointF(16, 12.5));
    } else if (n == "braces") {
        QFont f; f.setPixelSize(14); f.setBold(true); p.setFont(f);
        p.drawText(QRectF(0, 0, 20, 19), Qt::AlignCenter, "{ }");
    } else if (n == "server") {
        p.drawRoundedRect(QRectF(3, 3.5, 14, 5.2), 1.6, 1.6);
        p.drawRoundedRect(QRectF(3, 11.3, 14, 5.2), 1.6, 1.6);
        dot(6.2, 6.1, 0.9); dot(6.2, 13.9, 0.9);
    } else if (n == "sliders") {
        p.drawLine(QPointF(3, 6), QPointF(17, 6)); p.drawLine(QPointF(3, 14), QPointF(17, 14));
        p.setBrush(Qt::white); p.drawEllipse(QPointF(7, 6), 2.2, 2.2); p.drawEllipse(QPointF(13, 14), 2.2, 2.2);
    } else if (n == "pause") {
        p.drawEllipse(QRectF(2.5, 2.5, 15, 15));
        p.drawLine(QPointF(8, 7), QPointF(8, 13)); p.drawLine(QPointF(12, 7), QPointF(12, 13));
    } else if (n == "bolt") {
        QPolygonF b; b << QPointF(11, 2.5) << QPointF(5, 11) << QPointF(9.5, 11) << QPointF(8.6, 17.5)
                       << QPointF(15, 8.5) << QPointF(10.5, 8.5);
        p.drawPolygon(b);
    } else if (n == "warn") {
        QPolygonF t; t << QPointF(10, 3) << QPointF(17.5, 16.3) << QPointF(2.5, 16.3);
        p.drawPolygon(t);
        p.drawLine(QPointF(10, 8), QPointF(10, 12)); dot(10, 14.2, 0.8);
    } else if (n == "image") {
        p.drawRoundedRect(QRectF(3, 4, 14, 12), 2, 2); dot(7.2, 8, 1.2);
        QPainterPath m; m.moveTo(4, 14); m.lineTo(8, 10.6); m.lineTo(11, 13); m.lineTo(13.5, 11); m.lineTo(16, 14); p.drawPath(m);
    } else if (n == "target") {
        p.drawEllipse(QRectF(3.5, 3.5, 13, 13)); dot(10, 10, 1.7);
        p.drawLine(QPointF(10, 1.5), QPointF(10, 4)); p.drawLine(QPointF(10, 16), QPointF(10, 18.5));
        p.drawLine(QPointF(1.5, 10), QPointF(4, 10)); p.drawLine(QPointF(16, 10), QPointF(18.5, 10));
    } else if (n == "ruler") {
        p.drawRoundedRect(QRectF(2.5, 6.5, 15, 7), 1.2, 1.2);
        p.drawLine(QPointF(6, 6.5), QPointF(6, 10)); p.drawLine(QPointF(10, 6.5), QPointF(10, 11)); p.drawLine(QPointF(14, 6.5), QPointF(14, 10));
    } else if (n == "backspace") {
        QPolygonF b; b << QPointF(7, 4.5) << QPointF(17, 4.5) << QPointF(17, 15.5) << QPointF(7, 15.5) << QPointF(2.5, 10);
        p.drawPolygon(b);
        p.drawLine(QPointF(10, 8), QPointF(14, 12)); p.drawLine(QPointF(14, 8), QPointF(10, 12));
    } else if (n == "folder") {
        p.drawRoundedRect(QRectF(2.5, 6.5, 15, 9.5), 2, 2);
        QPainterPath t; t.moveTo(2.5, 7); t.lineTo(2.5, 5); t.quadTo(2.5, 4, 3.5, 4); t.lineTo(7.5, 4); t.lineTo(9.2, 6.5); p.drawPath(t);
    } else if (n == "bug") {
        p.drawEllipse(QRectF(6, 6, 8, 10));
        p.drawLine(QPointF(3, 8), QPointF(6.2, 9.2)); p.drawLine(QPointF(3, 13.5), QPointF(6.2, 12.3));
        p.drawLine(QPointF(17, 8), QPointF(13.8, 9.2)); p.drawLine(QPointF(17, 13.5), QPointF(13.8, 12.3));
        p.drawLine(QPointF(8, 3.5), QPointF(9, 6)); p.drawLine(QPointF(12, 3.5), QPointF(11, 6));
    } else if (n == "gauge") {
        p.drawArc(QRectF(2.5, 4.5, 15, 15), 0, 180 * 16);
        p.drawLine(QPointF(10, 12), QPointF(13.5, 8)); p.drawLine(QPointF(2.5, 12), QPointF(17.5, 12));
    } else if (n == "arrow") {
        p.drawLine(QPointF(4, 10), QPointF(16, 10));
        p.drawLine(QPointF(11.5, 5.5), QPointF(16, 10)); p.drawLine(QPointF(11.5, 14.5), QPointF(16, 10));
    }
    return pm;
}

QIcon makeIcon(const QString &name, const QColor &color, int size)
{
    return QIcon(iconPixmap(name, color, size));
}

QLabel *iconLabel(const QString &name, const QColor &color, int size)
{
    auto *l = new QLabel;
    l->setPixmap(iconPixmap(name, color, size));
    l->setFixedSize(size, size);
    l->setStyleSheet("background: transparent; border: none;");
    return l;
}
