#include "mainwindow.h"
#include "decodepage.h"
#include "generatepage.h"
#include "icons.h"
#include "widgets.h"

#include <QButtonGroup>
#include <QPainter>
#include <QScrollArea>
#include <QStackedWidget>

// brand logo tile (barcode glyph on a blue gradient)
class LogoTile : public QWidget {
public:
    explicit LogoTile(int s) { setFixedSize(s, s); }
protected:
    void paintEvent(QPaintEvent *) override
    {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);
        QLinearGradient g(0, 0, width(), height());
        g.setColorAt(0, QColor("#2F6BF0"));
        g.setColorAt(1, QColor("#0B3FB8"));
        p.setPen(Qt::NoPen);
        p.setBrush(g);
        p.drawRoundedRect(rect(), width() * 0.26, width() * 0.26);
        p.setBrush(Qt::white);
        const double u = width() / 32.0;
        const double xs[] = {8, 11.5, 14.5, 19, 22.5}, ws[] = {2, 1.2, 2.6, 1.2, 2};
        for (int i = 0; i < 5; ++i) p.drawRect(QRectF(xs[i] * u, 8.5 * u, ws[i] * u, 15 * u));
    }
};

static QScrollArea *scrollWrap(QWidget *page)
{
    auto *sa = new QScrollArea;
    sa->setWidgetResizable(true);
    sa->setFrameShape(QFrame::NoFrame);
    sa->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    sa->setWidget(page);
    return sa;
}

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
    setWindowTitle("BarcodeHub");
    resize(1360, 900);
    setMinimumSize(1180, 700);

    auto *root = new QWidget;
    root->setObjectName("root");
    setCentralWidget(root);
    auto *h = hbox(root, 0, 0, 0, 0, 0);
    h->addWidget(buildSidebar());

    auto *right = vcol(0);
    right->addWidget(buildTopBar());
    m_generate = new GeneratePage;
    m_decode = new DecodePage;
    m_pages = new QStackedWidget;
    m_pages->addWidget(scrollWrap(m_generate));
    m_pages->addWidget(scrollWrap(m_decode));
    right->addWidget(m_pages, 1);
    h->addLayout(right, 1);

    connect(m_decode, &DecodePage::generateFromCodeRequested, this, [this](const QString &code) {
        m_generate->setCode(code, true);
        m_nav->button(0)->click();
    });

    // ---------------------------------------------------------------------------
    // PLACEHOLDER (UI demo only) – delete once the real decoder exists.
    // Pressing "Decode Image" just alternates the success / failure states.
    // ---------------------------------------------------------------------------
    connect(m_decode, &DecodePage::decodeImageRequested, this, [this](const QString &) {
        static bool ok = false;
        if (ok) m_decode->showResult("749205"); else m_decode->showFailure();
        ok = !ok;
    });
}

QWidget *MainWindow::buildSidebar()
{
    auto *sb = new QFrame;
    sb->setObjectName("sidebar");
    sb->setFixedWidth(240);
    auto *l = vbox(sb, 0, 0, 0, 0, 0);

    auto *brand = new QFrame;
    brand->setObjectName("brandRow");
    brand->setFixedHeight(56);
    auto *bh = hbox(brand, 16, 0, 16, 0, 10);
    bh->addWidget(new LogoTile(32));
    auto *bt = vcol(-5);
    bt->addWidget(T("BarcodeHub", 16, 600, "#0B1C30"));
    bt->addWidget(T("Precision Utility", 11, 400, "#434655"));
    bh->addLayout(bt);
    bh->addStretch();
    l->addWidget(brand);

    auto *inner = new QWidget;
    auto *il = vbox(inner, 12, 12, 12, 12, 4);
    auto *cap = T("OPERATIONS", 11, 600, "#737686");
    cap->setContentsMargins(8, 4, 0, 4);
    il->addWidget(cap);

    m_nav = new QButtonGroup(this);
    m_nav->setExclusive(true);
    const QString names[] = {tr("Generate Barcode"), tr("Decode Barcode")};
    const QString icons[] = {"barcode", "scan"};
    for (int i = 0; i < 2; ++i) {
        auto *b = new QPushButton("  " + names[i]);
        b->setObjectName("nav");
        b->setCheckable(true);
        b->setCursor(Qt::PointingHandCursor);
        QIcon ic;
        ic.addPixmap(iconPixmap(icons[i], QColor("#434655"), 20), QIcon::Normal, QIcon::Off);
        ic.addPixmap(iconPixmap(icons[i], QColor("#004AC6"), 20), QIcon::Normal, QIcon::On);
        b->setIcon(ic);
        b->setIconSize(QSize(20, 20));
        m_nav->addButton(b, i);
        il->addWidget(b);
        if (i == 0) b->setChecked(true);
    }
    connect(m_nav, &QButtonGroup::idClicked, this, [this](int id) { m_pages->setCurrentIndex(id); });
    il->addStretch();
    l->addWidget(inner, 1);

    auto *foot = new QWidget;
    auto *fl = vbox(foot, 12, 0, 12, 14, 0);
    auto *chipRow = panel();
    auto *cr = hbox(chipRow, 12, 9, 12, 9, 8);
    cr->addWidget(T("\xE2\x97\x8F", 9, 400, "#0B4BCB"));
    cr->addWidget(T("Engine v2.4", 11, 500, "#434655", true));
    cr->addStretch();
    cr->addWidget(T("1440x900", 11, 400, "#737686", true));
    fl->addWidget(chipRow);
    l->addWidget(foot);
    return sb;
}

QWidget *MainWindow::buildTopBar()
{
    auto *tb = new QFrame;
    tb->setObjectName("topbar");
    tb->setFixedHeight(56);
    auto *l = hbox(tb, 24, 0, 24, 0, 12);
    l->addWidget(new LogoTile(30), 0, Qt::AlignVCenter);
    l->addWidget(T("BarcodeHub", 17, 600, "#0B1C30"), 0, Qt::AlignVCenter);
    auto *sep = new QFrame;
    sep->setFixedSize(1, 20);
    sep->setStyleSheet("background:#DDE2EC; border:none;");
    l->addWidget(sep, 0, Qt::AlignVCenter);
    l->addWidget(pill("\xE2\x97\x8F  Increment 1", "tint"), 0, Qt::AlignVCenter);
    l->addStretch();
    l->addWidget(iconChip("server", "Hardware Link Active", "#EFF4FF", "#434655", "#E1E8F7", true, 12, 8), 0, Qt::AlignVCenter);
    l->addSpacing(8);
    auto *av = new QFrame;
    av->setObjectName("avatar");
    av->setFixedSize(40, 40);
    auto *al = hbox(av, 0, 0, 0, 0, 0);
    al->setAlignment(Qt::AlignCenter);
    al->addWidget(iconLabel("user", QColor("#FFFFFF"), 20));
    l->addWidget(av, 0, Qt::AlignVCenter);
    return tb;
}
