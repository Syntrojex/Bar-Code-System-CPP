#include "widgets.h"
#include "icons.h"

#include <QDragEnterEvent>
#include <QDropEvent>
#include <QFileDialog>
#include <QFileInfo>
#include <QMimeData>
#include <QPainter>
#include <QPainterPath>
#include <QStyle>
#include <QtMath>

// ======================================================= helpers
QVBoxLayout *vbox(QWidget *w, int l, int t, int r, int b, int s)
{
    auto *v = new QVBoxLayout(w);
    v->setContentsMargins(l, t, r, b);
    v->setSpacing(s);
    return v;
}
QHBoxLayout *hbox(QWidget *w, int l, int t, int r, int b, int s)
{
    auto *h = new QHBoxLayout(w);
    h->setContentsMargins(l, t, r, b);
    h->setSpacing(s);
    return h;
}
QHBoxLayout *hrow(int s) { auto *h = new QHBoxLayout; h->setContentsMargins(0, 0, 0, 0); h->setSpacing(s); return h; }
QVBoxLayout *vcol(int s) { auto *v = new QVBoxLayout; v->setContentsMargins(0, 0, 0, 0); v->setSpacing(s); return v; }

QString monoFamily() { return QStringLiteral("'JetBrains Mono','Consolas','DejaVu Sans Mono',monospace"); }

QFont monoFont(int px, bool bold)
{
    QFont f;
    f.setFamilies({"JetBrains Mono", "Consolas", "DejaVu Sans Mono", "Courier New"});
    f.setStyleHint(QFont::Monospace);
    f.setPixelSize(px);
    f.setBold(bold);
    return f;
}

QLabel *T(const QString &text, int px, int weight, const QString &color, bool mono)
{
    auto *l = new QLabel(text);
    QString css = QString("font-size:%1px; font-weight:%2; color:%3;").arg(px).arg(weight).arg(color);
    if (mono) css += "font-family:" + monoFamily() + ";";
    l->setStyleSheet(css);
    return l;
}

QLabel *caption(const QString &text, bool mono) { return T(text, 11, 600, "#737686", mono); }

// Pills and chips are painted by hand: Qt stylesheets drop border-radius on QLabel when
// the colour comes from a dynamic-property rule, so custom painting is the reliable way.
class Badge : public QLabel {
public:
    Badge(const QString &text, const QString &kind, bool isPill) : QLabel(text), m_pill(isPill)
    {
        setProperty("kind", kind);
        QFont f = isPill ? QFont() : monoFont(11, true);
        f.setPixelSize(isPill ? 12 : 11);
        f.setWeight(QFont::DemiBold);
        setFont(f);
        setContentsMargins(isPill ? 11 : 8, isPill ? 3 : 2, isPill ? 11 : 8, isPill ? 3 : 2);
        restyle();
    }
protected:
    bool event(QEvent *e) override
    {
        if (e->type() == QEvent::DynamicPropertyChange) restyle();
        return QLabel::event(e);
    }
    void paintEvent(QPaintEvent *e) override
    {
        {
            QPainter p(this);
            p.setRenderHint(QPainter::Antialiasing);
            p.setBrush(m_bg);
            p.setPen(m_border.isValid() ? QPen(m_border, 1) : QPen(Qt::NoPen));
            const qreal r = m_pill ? height() / 2.0 : 6;
            p.drawRoundedRect(QRectF(rect()).adjusted(0.5, 0.5, -0.5, -0.5), r, r);
        }
        QLabel::paintEvent(e);
    }
private:
    void restyle()
    {
        const QString k = property("kind").toString();
        m_border = QColor();
        if (m_pill) {
            if (k == "white")      { m_bg = "#FFFFFF"; m_fg = "#0B1C30"; m_border = QColor("#E4E8F1"); }
            else if (k == "green") { m_bg = "#E3F6EC"; m_fg = "#0B7A4B"; }
            else if (k == "red")   { m_bg = "#FFDAD6"; m_fg = "#93000A"; }
            else if (k == "idle")  { m_bg = "#EEF2FA"; m_fg = "#5A6075"; }
            else                   { m_bg = "#E1EAFF"; m_fg = "#1D4ED8"; }      // tint
        } else {
            if (k == "neutral")    { m_bg = "#EEF2FA"; m_fg = "#434655"; m_border = QColor("#E2E7F1"); }
            else if (k == "white") { m_bg = "#FFFFFF"; m_fg = "#434655"; }
            else if (k == "green") { m_bg = "#E3F6EC"; m_fg = "#0B7A4B"; }
            else                   { m_bg = "#E1EAFF"; m_fg = "#1F4FCF"; }      // blue
        }
        setStyleSheet(QString("color:%1; background:transparent; border:none;").arg(m_fg.name()));
        update();
    }
    bool m_pill;
    QColor m_bg, m_fg, m_border;
};

QLabel *chip(const QString &text, const char *kind) { return new Badge(text, kind, false); }
QLabel *pill(const QString &text, const char *kind) { return new Badge(text, kind, true); }
QLabel *monoPill(const QString &text)
{
    auto *l = new QLabel(text);
    l->setObjectName("monoPill");
    return l;
}

QFrame *card()  { auto *f = new QFrame; f->setObjectName("card"); return f; }
QFrame *panel(const char *name) { auto *f = new QFrame; f->setObjectName(name); return f; }
QFrame *hline() { auto *f = new QFrame; f->setObjectName("hline"); f->setFixedHeight(1); return f; }

QFrame *iconTile(const QString &icon, const QColor &color, int box, int iconSize)
{
    auto *f = new QFrame;
    f->setObjectName("iconTile");
    f->setFixedSize(box, box);
    f->setStyleSheet(QString("QFrame#iconTile { background:#E5EEFF; border:none; border-radius:%1px; }").arg(box / 4));
    auto *l = hbox(f, 0, 0, 0, 0, 0);
    l->setAlignment(Qt::AlignCenter);
    l->addWidget(iconLabel(icon, color, iconSize));
    return f;
}

void setKind(QWidget *w, const char *kind)
{
    w->setProperty("kind", kind);
    w->style()->unpolish(w);
    w->style()->polish(w);
    w->update();
}

void setBadge(QLabel *l, const QString &text, const char *kind)
{
    l->setText(text);
    setKind(l, kind);
}

QPushButton *btn(const QString &text, const char *kind, const QString &icon)
{
    auto *b = new QPushButton((icon.isEmpty() || text.isEmpty()) ? text : " " + text);
    b->setObjectName("btn");
    b->setProperty("kind", kind);
    b->setCursor(Qt::PointingHandCursor);
    if (!icon.isEmpty()) {
        const QString k = kind;
        QColor c("#334155");
        if (k == "primary" || k == "deep") c = QColor("#FFFFFF");
        else if (k == "tint") c = QColor("#1D4ED8");
        else if (k == "link") c = QColor("#004AC6");
        else if (k == "danger") c = QColor("#BA1A1A");
        b->setIcon(makeIcon(icon, c, 18));
        b->setIconSize(QSize(18, 18));
    }
    return b;
}

// ======================================================= mock bars
void drawMockBars(QPainter &p, const QRect &r, const QString &codeIn, const QColor &color)
{
    const QString code = codeIn.isEmpty() ? QString("000000") : codeIn;
    const bool dense = r.width() >= 140;              // small glyphs use fewer, thicker bars
    QVector<int> w = {2, 1, 1, 2, 3, 2};                                    // start
    for (int i = 0; i < code.size(); ++i) {
        int d = code[i].digitValue();
        if (d < 0) d = 0;
        w << (d % 3) + 1 << ((d * 7 + i) % 3) + 1 << ((d + i * 2) % 2) + 1
          << ((d * 5 + 3) % 3) + 1 << ((d + i) % 2) + 1 << ((d * 3 + 1) % 3) + 1
;
        if (dense)
            w << ((d * 2 + i) % 3) + 1 << ((d + i * 3) % 2) + 1 << ((d * 4 + 1) % 3) + 1
              << ((d + 2 * i) % 2) + 1 << ((d * 6 + i) % 3) + 1 << ((d + 1) % 2) + 1;
    }
    w << 2 << 3 << 3 << 1 << 1 << 1 << 2;                                   // stop
    int total = 0;
    for (int v : w) total += v;

    p.save();
    p.setRenderHint(QPainter::Antialiasing, false);
    p.setPen(Qt::NoPen);
    p.setBrush(color);
    const double unit = double(r.width()) / total;
    double x = r.left();
    for (int i = 0; i < w.size(); ++i) {
        const double bw = w[i] * unit;
        if (i % 2 == 0)
            p.drawRect(QRect(qRound(x), r.top(), qMax(1, qRound(x + bw) - qRound(x)), r.height()));
        x += bw;
    }
    p.restore();
}

static QString spaced(const QString &s)
{
    QStringList parts;
    for (QChar c : s) parts << QString(c);
    return parts.join("  ");
}

MiniBarcode::MiniBarcode(const QString &code, QSize size, QWidget *parent) : QWidget(parent), m_code(code)
{
    setFixedSize(size);
}
void MiniBarcode::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    drawMockBars(p, rect(), m_code, QColor("#0B1C30"));
}

// ======================================================= BarcodeView (Generate stage)
BarcodeView::BarcodeView(QWidget *parent) : QWidget(parent)
{
    setMinimumHeight(330);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
}
void BarcodeView::setCode(const QString &c)  { m_code = c; m_image = QPixmap(); update(); }
void BarcodeView::setImage(const QPixmap &p) { m_image = p; update(); }
void BarcodeView::setShowDigits(bool on)     { m_digits = on; update(); }
void BarcodeView::setAwaiting(bool on)       { m_awaiting = on; update(); }
void BarcodeView::clear()                    { m_code.clear(); m_image = QPixmap(); update(); }

void BarcodeView::paintBarcodeCard(QPainter &p, const QRect &r) const
{
    p.fillRect(r, Qt::white);
    if (!m_image.isNull()) {
        const QPixmap s = m_image.scaled(r.size() - QSize(40, 24), Qt::KeepAspectRatio, Qt::SmoothTransformation);
        p.drawPixmap(r.center().x() - s.width() / 2, r.center().y() - s.height() / 2, s);
        return;
    }
    const int mx = qRound(r.width() * 0.10);
    const int barH = m_digits ? r.height() - 90 : r.height() - 56;
    drawMockBars(p, QRect(r.left() + mx, r.top() + 28, r.width() - 2 * mx, barH), m_code, QColor("#0B1C30"));
    if (m_digits) {
        QFont f = monoFont(22, true);
        f.setLetterSpacing(QFont::AbsoluteSpacing, 0);
        p.setFont(f);
        p.setPen(QColor("#0B1C30"));
        p.drawText(QRect(r.left(), r.top() + 28 + barH + 8, r.width(), 36), Qt::AlignCenter, spaced(m_code));
    }
}

QPixmap BarcodeView::renderBarcode() const
{
    const QRect base(0, 0, 450, 180);
    QPixmap pm(base.size() * 2);
    pm.fill(Qt::white);
    QPainter p(&pm);
    p.setRenderHint(QPainter::TextAntialiasing);
    p.scale(2, 2);
    paintBarcodeCard(p, base);
    return pm;
}

void BarcodeView::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    // stage + dot grid
    QPainterPath stage;
    stage.addRoundedRect(QRectF(rect()), 10, 10);
    p.fillPath(stage, QColor("#EEF3FF"));
    p.setClipPath(stage);
    p.setPen(QPen(QColor("#C9D6EE"), 1.5, Qt::SolidLine, Qt::RoundCap));
    for (int y = 12; y < height(); y += 16)
        for (int x = 12; x < width(); x += 16) p.drawPoint(x, y);
    p.setClipping(false);

    const QColor muted("#737686");
    QFont mf = monoFont(11);
    p.setFont(mf);

    // ruler row (top)
    p.drawPixmap(14, 16, iconPixmap("ruler", muted, 14));
    p.setPen(muted);
    p.drawText(QRect(32, 12, 150, 22), Qt::AlignVCenter | Qt::AlignLeft, "600 px width");
    p.drawText(QRect(width() - 182, 12, 150, 22), Qt::AlignVCenter | Qt::AlignRight, "240 px height");
    p.drawPixmap(width() - 28, 16, iconPixmap("ruler", muted, 14));

    const QString sym = "Active Symbology: CODE 128";
    const int sw = QFontMetrics(mf).horizontalAdvance(sym) + 22;
    const QRect symR((width() - sw) / 2, 11, sw, 24);
    p.setPen(Qt::NoPen);
    p.setBrush(QColor("#E3EAF8"));
    p.drawRoundedRect(symR, 6, 6);
    p.setPen(QColor("#434655"));
    p.drawText(symR, Qt::AlignCenter, sym);

    // bottom row
    p.setBrush(QColor("#0B4BCB"));
    p.setPen(Qt::NoPen);
    p.drawEllipse(QPoint(20, height() - 22), 3, 3);
    p.setPen(muted);
    p.drawText(QRect(30, height() - 34, 220, 22), Qt::AlignVCenter | Qt::AlignLeft, "Optical Density: 99.4%");
    p.drawText(QRect(width() - 262, height() - 34, 246, 22), Qt::AlignVCenter | Qt::AlignRight, "Standard Bumper: Left/Right 12mm");

    // center card
    const int cw = qMin(width() - 80, 450), ch = 236;
    const QRect c((width() - cw) / 2, (height() - ch) / 2, cw, ch);

    if (!hasBarcode() || m_awaiting) {
        p.setPen(QPen(QColor("#9DB4E0"), 1.6, Qt::DashLine));
        p.setBrush(QColor(255, 255, 255, 170));
        p.drawRoundedRect(c, 10, 10);
        QFont f; f.setPixelSize(15); f.setWeight(QFont::DemiBold);
        p.setFont(f);
        p.setPen(QColor("#434655"));
        p.drawText(c.adjusted(0, 0, 0, -22), Qt::AlignCenter, tr("Your barcode will appear here"));
        f.setPixelSize(12); f.setWeight(QFont::Normal);
        p.setFont(f);
        p.setPen(QColor("#737686"));
        p.drawText(c.adjusted(0, 34, 0, 0), Qt::AlignCenter, tr("Enter a 6-digit code and press Generate"));
        return;
    }

    for (int i = 5; i >= 1; --i) {                                          // soft shadow
        p.setPen(Qt::NoPen);
        p.setBrush(QColor(15, 23, 42, 6));
        p.drawRoundedRect(c.adjusted(-i, -i + 4, i, i + 4), 8 + i, 8 + i);
    }
    p.setBrush(Qt::white);
    p.setPen(QPen(QColor("#E4E8F1"), 1));
    p.drawRoundedRect(c, 8, 8);
    p.save();
    p.setRenderHint(QPainter::Antialiasing, false);
    paintBarcodeCard(p, QRect(c.left() + 1, c.top() + 1, c.width() - 2, 180));
    p.restore();

    // footer inside card
    p.setFont(monoFont(11));
    const QString t1 = "Hardware Decodable", t2 = "PAYLOAD: " + m_code;
    const QFontMetrics fm(p.font());
    const int w1 = fm.horizontalAdvance(t1), w2 = fm.horizontalAdvance(t2);
    const int total = 18 + w1 + 22 + w2;
    int x = c.left() + (c.width() - total) / 2;
    const int y = c.top() + 196;
    p.drawPixmap(x, y + 2, iconPixmap("scan", QColor("#0B4BCB"), 14));
    p.setPen(QColor("#0B4BCB"));
    p.drawText(QRect(x + 18, y, w1 + 4, 20), Qt::AlignVCenter, t1);
    p.setBrush(QColor("#C3C6D7")); p.setPen(Qt::NoPen);
    p.drawEllipse(QPoint(x + 18 + w1 + 12, y + 10), 2, 2);
    p.setPen(QColor("#0B1C30"));
    p.drawText(QRect(x + 18 + w1 + 22, y, w2 + 4, 20), Qt::AlignVCenter, t2);
}

// ======================================================= CameraView
CameraView::CameraView(QWidget *parent) : QWidget(parent)
{
    setFixedHeight(352);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_timer.setInterval(16);
    connect(&m_timer, &QTimer::timeout, this, [this] { m_phase += 0.03; update(); });
    m_timer.start();
}
void CameraView::setRunning(bool on) { m_running = on; if (on && !m_frozen) m_timer.start(); else m_timer.stop(); update(); }
void CameraView::setFrozen(bool on)  { m_frozen = on; if (on) m_timer.stop(); else if (m_running) m_timer.start(); update(); }
void CameraView::setPreviewCode(const QString &d) { m_code = d; update(); }
void CameraView::setFrame(const QPixmap &pm) { m_frame = pm; update(); }

static void drawChip(QPainter &p, const QRect &r, const QString &text, const QColor &bg, const QColor &fg,
                     const QColor &dot = QColor(), const QString &icon = QString())
{
    p.setPen(Qt::NoPen);
    p.setBrush(bg);
    p.drawRoundedRect(r, 6, 6);
    int x = r.left() + 10;
    if (dot.isValid()) { p.setBrush(dot); p.drawEllipse(QPoint(x + 3, r.center().y()), 4, 4); x += 14; }
    if (!icon.isEmpty()) { p.drawPixmap(x, r.center().y() - 7, iconPixmap(icon, fg, 14)); x += 20; }
    p.setPen(fg);
    p.drawText(QRect(x, r.top(), r.right() - x, r.height()), Qt::AlignVCenter | Qt::AlignLeft, text);
}

void CameraView::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    QPainterPath clip;
    clip.addRoundedRect(QRectF(rect()), 10, 10);
    p.setClipPath(clip);

    QRadialGradient bg(width() / 2.0, height() / 2.0, qMax(width(), height()) * 0.7);
    bg.setColorAt(0, QColor("#10243F"));
    bg.setColorAt(1, QColor("#070F1C"));
    p.fillRect(rect(), bg);
    if (!m_frame.isNull())
        p.drawPixmap(rect(), m_frame.scaled(size(), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation));

    p.setPen(QPen(QColor(255, 255, 255, 14), 1.5, Qt::SolidLine, Qt::RoundCap));
    for (int y = 14; y < height(); y += 18)
        for (int x = 14; x < width(); x += 18) p.drawPoint(x, y);

    // scan frame (corner brackets)
    const int fw = int(width() * 0.74), fh = int(height() * 0.64);
    const QRect f((width() - fw) / 2, (height() - fh) / 2, fw, fh);
    p.setBrush(Qt::NoBrush);
    p.setPen(QPen(QColor("#1B52D6"), 4, Qt::SolidLine, Qt::FlatCap, Qt::MiterJoin));
    const int L = 30;
    auto corner = [&](QPoint c, int dx, int dy) {
        p.drawLine(c, c + QPoint(dx * L, 0));
        p.drawLine(c, c + QPoint(0, dy * L));
    };
    corner(f.topLeft() + QPoint(2, 2), 1, 1);
    corner(f.topRight() + QPoint(-2, 2), -1, 1);
    corner(f.bottomLeft() + QPoint(2, -2), 1, -1);
    corner(f.bottomRight() + QPoint(-2, -2), -1, -1);

    // sample barcode card
    const int cw = int(width() * 0.62), ch = int(height() * 0.47);
    const QRect bc((width() - cw) / 2, (height() - ch) / 2, cw, ch);
    p.setPen(Qt::NoPen);
    p.setBrush(Qt::white);
    p.drawRoundedRect(bc, 10, 10);
    const int mx = int(bc.width() * 0.09);
    drawMockBars(p, QRect(bc.left() + mx, bc.top() + 22, bc.width() - 2 * mx, bc.height() - 70), m_code, QColor("#0B1C30"));
    QFont df = monoFont(15, true);
    p.setFont(df);
    p.setPen(QColor("#0B1C30"));
    p.drawText(QRect(bc.left(), bc.bottom() - 40, bc.width(), 30), Qt::AlignCenter, spaced(m_code));

    // scan line
    const qreal t = m_frozen ? 0.5 : (qSin(m_phase) + 1) / 2;
    const int y = f.top() + 14 + int(t * (f.height() - 28));
    QLinearGradient glow(0, y - 26, 0, y + 26);
    glow.setColorAt(0, QColor(34, 211, 238, 0));
    glow.setColorAt(0.5, QColor(34, 211, 238, 55));
    glow.setColorAt(1, QColor(34, 211, 238, 0));
    p.fillRect(QRect(f.left(), y - 26, f.width(), 52), glow);
    QLinearGradient ln(f.left(), 0, f.right(), 0);
    ln.setColorAt(0, QColor(34, 211, 238, 0));
    ln.setColorAt(0.5, QColor(103, 232, 249, 255));
    ln.setColorAt(1, QColor(34, 211, 238, 0));
    p.setPen(QPen(QBrush(ln), 2));
    p.drawLine(f.left(), y, f.right(), y);

    // HUD chips
    p.setFont(monoFont(11));
    drawChip(p, QRect(14, 14, 252, 26), "LIVE SENSOR: 1920x1080@60Hz", QColor(0, 0, 0, 110), QColor("#E2E8F0"),
             m_running ? QColor("#34D399") : QColor("#94A3B8"));
    drawChip(p, QRect(width() - 202, 14, 188, 26), "ROI: ACTIVE [460x220]", QColor(0, 0, 0, 110), QColor("#CFE0FF"),
             QColor(), "target");
    drawChip(p, QRect(width() - 224, height() - 40, 210, 26), "AGC: +2.4dB | EXP: 1/120s", QColor(0, 0, 0, 120), QColor("#CBD5E1"));
}

// ======================================================= DropZone
DropZone::DropZone(QWidget *parent) : QFrame(parent)
{
    setAcceptDrops(true);
    setCursor(Qt::PointingHandCursor);
    setFixedHeight(216);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    auto *lay = vbox(this, 20, 16, 20, 16, 6);
    lay->setAlignment(Qt::AlignCenter);

    auto *tile = iconTile("upload", QColor("#0B4BCB"), 56, 26);
    tile->setStyleSheet("QFrame#iconTile { background:#DCE8FF; border:none; border-radius:14px; }");
    lay->addWidget(tile, 0, Qt::AlignHCenter);
    lay->addSpacing(8);

    auto *t = T(tr("Drag & drop a barcode image here or click to browse"), 18, 500, "#0B1C30");
    t->setAlignment(Qt::AlignCenter);
    t->setWordWrap(true);
    lay->addWidget(t);
    auto *s = T(tr("High-contrast raster images yield optimal decoding accuracy"), 14, 400, "#434655");
    s->setAlignment(Qt::AlignCenter);
    s->setWordWrap(true);
    lay->addWidget(s);
    lay->addSpacing(8);

    auto *row = hrow(8);
    row->addStretch();
    for (const char *f : {"PNG", "JPG", "JPEG", "BMP"}) row->addWidget(chip(f, "white"));
    row->addWidget(T("(Up to 10MB)", 11, 400, "#737686", true));
    row->addStretch();
    lay->addLayout(row);
}

void DropZone::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    p.setBrush(m_hover ? QColor("#E6EEFF") : QColor("#EFF4FF"));
    p.setPen(QPen(m_hover ? QColor("#0B4BCB") : QColor("#CBDAF7"), 1.5, Qt::DashLine));
    p.drawRoundedRect(QRectF(rect()).adjusted(1, 1, -1, -1), 12, 12);
}

bool DropZone::event(QEvent *e)
{
    if (e->type() == QEvent::Enter) { m_hover = true; update(); }
    else if (e->type() == QEvent::Leave) { m_hover = false; update(); }
    return QFrame::event(e);
}

void DropZone::mousePressEvent(QMouseEvent *)
{
    const QString path = QFileDialog::getOpenFileName(this, tr("Select barcode image"), QString(),
                                                      tr("Images (*.png *.jpg *.jpeg *.bmp)"));
    if (!path.isEmpty()) emit fileChosen(path);
}
void DropZone::dragEnterEvent(QDragEnterEvent *e)
{
    if (e->mimeData()->hasUrls()) { m_hover = true; update(); e->acceptProposedAction(); }
}
void DropZone::dragLeaveEvent(QDragLeaveEvent *) { m_hover = false; update(); }
void DropZone::dropEvent(QDropEvent *e)
{
    m_hover = false; update();
    for (const QUrl &u : e->mimeData()->urls()) {
        const QString suf = QFileInfo(u.toLocalFile()).suffix().toLower();
        if (u.isLocalFile() && (suf == "png" || suf == "jpg" || suf == "jpeg" || suf == "bmp")) {
            emit fileChosen(u.toLocalFile());
            return;
        }
    }
}

// ======================================================= ToggleSwitch
ToggleSwitch::ToggleSwitch(bool on, QWidget *parent) : QWidget(parent), m_on(on)
{
    setFixedSize(44, 24);
    setCursor(Qt::PointingHandCursor);
}
void ToggleSwitch::setChecked(bool on) { if (on != m_on) { m_on = on; update(); emit toggled(on); } }
void ToggleSwitch::mouseReleaseEvent(QMouseEvent *) { setChecked(!m_on); }
void ToggleSwitch::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    p.setPen(Qt::NoPen);
    p.setBrush(m_on ? QColor("#2563EB") : QColor("#C3CBDB"));
    p.drawRoundedRect(rect(), 12, 12);
    p.setBrush(Qt::white);
    p.drawEllipse(QPoint(m_on ? width() - 12 : 12, height() / 2), 9, 9);
}

// ======================================================= RadioCard
RadioCard::RadioCard(const QString &title, const QString &sub, QWidget *parent) : QFrame(parent)
{
    setCursor(Qt::PointingHandCursor);
    setMinimumHeight(80);
    auto *v = vbox(this, 14, 12, 40, 12, 3);
    v->addWidget(T(title, 14, 600, "#0B1C30"));
    v->addWidget(T(sub, 11, 400, "#737686", true));
    v->addStretch();
}
void RadioCard::setSelected(bool on) { m_sel = on; update(); }
void RadioCard::mouseReleaseEvent(QMouseEvent *) { emit clicked(); }
void RadioCard::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    p.setBrush(m_sel ? QColor("#FFFFFF") : QColor("#E3EBFA"));
    p.setPen(m_sel ? QPen(QColor("#DDE5F5"), 1) : Qt::NoPen);
    p.drawRoundedRect(QRectF(rect()).adjusted(0.5, 0.5, -0.5, -0.5), 8, 8);
    const QPoint c(width() - 22, 26);
    p.setBrush(Qt::NoBrush);
    p.setPen(QPen(m_sel ? QColor("#0B4BCB") : QColor("#8A93A8"), 1.8));
    p.drawEllipse(c, 8, 8);
    if (m_sel) { p.setPen(Qt::NoPen); p.setBrush(QColor("#0B4BCB")); p.drawEllipse(c, 4, 4); }
}

// ======================================================= SegBar
SegBar::SegBar(const QStringList &labels, const QStringList &icons, int current, bool small, QWidget *parent)
    : QFrame(parent), m_icons(icons), m_current(current)
{
    setObjectName("segBar");
    setProperty("small", small);
    auto *l = hbox(this, 4, 4, 4, 4, 4);
    for (int i = 0; i < labels.size(); ++i) {
        auto *b = new QPushButton(icons.value(i).isEmpty() ? labels[i] : "  " + labels[i]);
        b->setObjectName("seg");
        b->setCheckable(true);
        b->setCursor(Qt::PointingHandCursor);
        b->setIconSize(QSize(18, 18));
        connect(b, &QPushButton::clicked, this, [this, i] { setCurrent(i); emit currentChanged(i); });
        l->addWidget(b);
        m_btns << b;
    }
    setCurrent(current);
}

void SegBar::setCurrent(int i)
{
    m_current = i;
    for (int k = 0; k < m_btns.size(); ++k) m_btns[k]->setChecked(k == i);
    refreshIcons();
}

void SegBar::refreshIcons()
{
    for (int k = 0; k < m_btns.size(); ++k) {
        if (m_icons.value(k).isEmpty()) continue;
        m_btns[k]->setIcon(makeIcon(m_icons[k], k == m_current ? QColor("#004AC6") : QColor("#434655"), 18));
    }
}

QFrame *iconChip(const QString &icon, const QString &text, const QString &bg, const QString &fg,
                 const QString &border, bool mono, int px, int radius)
{
    auto *f = new QFrame;
    f->setObjectName("iconChip");
    QString css = QString("QFrame#iconChip { background:%1; ").arg(bg);
    css += border.isEmpty() ? "border:none; " : QString("border:1px solid %1; ").arg(border);
    css += QString("border-radius:%1px; }").arg(radius);        // radius last: a later border shorthand would reset it
    f->setStyleSheet(css);
    auto *l = hbox(f, 10, 5, 12, 5, 7);
    l->addWidget(iconLabel(icon, QColor(fg), 15));
    l->addWidget(T(text, px, 600, fg, mono));
    return f;
}
