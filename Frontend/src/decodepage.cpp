#include "decodepage.h"
#include "icons.h"
#include "widgets.h"

#include <QApplication>
#include <QClipboard>
#include <QComboBox>
#include <QFileInfo>
#include <QGridLayout>   // used by the stat/tile grids
#include <QPainter>
#include <QProgressBar>
#include <QRegularExpression>
#include <QStackedWidget>
#include <QTime>
#include <QTimer>
#include <QCheckBox>
#include <QFileDialog>
#include <QHeaderView>
#include <QMessageBox>
#include <QSaveFile>
#include <QTableWidget>
#include <QTextStream>
#include <QDateTime>

// ------------------------------------------------------------------ local helpers
static QLabel *bigCode(const QString &text, int px, const QString &color)
{
    auto *l = new QLabel(text);
    QFont f = monoFont(px, true);
    f.setLetterSpacing(QFont::AbsoluteSpacing, px / 10.0);
    l->setFont(f);
    l->setStyleSheet("color:" + color + ";");
    l->setAlignment(Qt::AlignCenter);
    return l;
}

static QHBoxLayout *kv(const QString &k, QWidget *v, int px = 14)
{
    auto *h = hrow(8);
    h->addWidget(T(k, px, 400, "#434655"));
    h->addStretch();
    h->addWidget(v);
    return h;
}

// Project rule: exactly six digits, first digit must not be zero.
static bool isValidCode(const QString &s)
{
    static const QRegularExpression re("^[1-9][0-9]{5}$");
    return re.match(s).hasMatch();
}

static void wireCopy(QPushButton *b, QLabel *code)
{
    QObject::connect(b, &QPushButton::clicked, b, [b, code] {
        if (b->property("copied").toBool()) return;     // double-click guard
        QApplication::clipboard()->setText(code->text());
        const QString old = b->text();
        b->setProperty("copied", true);
        b->setText(" Copied!");
        QTimer::singleShot(1300, b, [b, old] {
            b->setText(old);
            b->setProperty("copied", false);
        });
    });
}

static QFrame *resultHeader(const QString &title, QLabel *badge, int px = 18)
{
    auto *f = new QFrame;
    auto *h = hbox(f, 0, 0, 0, 0, 10);
    h->addWidget(iconLabel("check", QColor("#0B4BCB"), px >= 18 ? 22 : 20));
    h->addWidget(T(title, px, 600, "#0B1C30"));
    h->addStretch();
    h->addWidget(badge);
    return f;
}

static QPixmap miniBarPixmap(const QString &code, QSize s)
{
    QPixmap pm(s);
    pm.fill(Qt::white);
    QPainter p(&pm);
    drawMockBars(p, QRect(8, 8, s.width() - 16, s.height() - 16), code, QColor("#0B1C30"));
    return pm;
}

// ------------------------------------------------------------------ construction
DecodePage::DecodePage(QWidget *parent) : QWidget(parent)
{
    setObjectName("page");
    auto *l = vbox(this, 0, 0, 0, 0, 0);
    m_stack = new QStackedWidget;
    m_stack->addWidget(buildCameraPage());
    m_stack->addWidget(buildUploadPage());
    m_stack->addWidget(buildLogsPage());
    l->addWidget(m_stack);
    m_loaded->hide();                                // nothing loaded yet
    resetResult();                                   // no demo data before the first real scan
    setCameraList({});
}

void DecodePage::setTab(int i)
{
    if (i != 0) stopCamera();                        // never keep the webcam on while another tab is shown
    m_stack->setCurrentIndex(i);
    for (auto *tabs : m_tabs) tabs->setCurrent(i);
}

SegBar *DecodePage::makeTabs(int current)
{
    auto *bar = new SegBar({tr("Scan with Camera"), tr("Upload Image"), tr("Scan Logs")},
                           {"camera", "upload", "history"}, current);
    connect(bar, &SegBar::currentChanged, this, &DecodePage::setTab);
    m_tabs << bar;
    return bar;
}

QFrame *DecodePage::makeFailBox()
{
    auto *f = panel("fail");
    auto *v = vbox(f, 16, 14, 16, 14, 8);
    auto *top = hrow();
    top->addWidget(iconChip("warn", tr("Could not read barcode"), "#FFDAD6", "#93000A", QString(), false, 12, 6));
    top->addStretch();
    top->addWidget(T("Telemetry Exception", 11, 400, "#737686", true));
    v->addLayout(top);
    auto *t = T(tr("Could not decode the barcode."), 14, 400, "#0B1C30");
    t->setObjectName("failReason");
    t->setWordWrap(true);
    v->addWidget(t);
    auto *b = hrow();
    b->addWidget(T(tr("Troubleshoot Optical Quality Guide  \xE2\x86\x92"), 12, 700, "#BA1A1A"));
    b->addStretch();
    auto *errLbl = T("ERR_UNKNOWN", 10, 400, "#737686", true);
    errLbl->setObjectName("failCode");
    b->addWidget(errLbl);
    v->addLayout(b);
    return f;
}

// ================================================================== screen 2 – camera
QWidget *DecodePage::buildCameraPage()
{
    auto *page = new QWidget;
    page->setObjectName("page");
    auto *root = vbox(page, 24, 22, 24, 24, 18);

    auto *head = hrow(14);
    head->addWidget(iconTile("scan", QColor("#0B4BCB"), 48, 24), 0, Qt::AlignTop);
    auto *tt = vcol(4);
    auto *tr1 = hrow(12);
    tr1->addWidget(T(tr("Decode Barcode"), 26, 600, "#0B1C30"));
    tr1->addWidget(chip("Increment 1", "blue"));
    tr1->addStretch();
    tt->addLayout(tr1);
    tt->addWidget(T(tr("Real-time hardware optical recognition & industrial symbology parsing"), 14, 400, "#434655"));
    head->addLayout(tt);
    head->addStretch();
    m_engine = monoPill("ENGINE: Idle");
    head->addWidget(m_engine, 0, Qt::AlignTop);
    root->addLayout(head);
    root->addWidget(makeTabs(0), 0, Qt::AlignLeft);

    auto *body = hrow(20);

    // ---- left card
    auto *left = card();
    auto *ll = vbox(left, 20, 20, 20, 20, 14);

    auto *bar = panel();
    auto *bl = hbox(bar, 14, 10, 14, 10, 10);
    bl->addWidget(T("CAPTURE INPUT:", 11, 500, "#737686", true));
    m_cameraBox = new QComboBox;
    m_cameraBox->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLengthWithIcon);
    m_cameraBox->setMinimumContentsLength(10);
    m_cameraBox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    bl->addWidget(m_cameraBox, 1);
    m_stream = btn(tr("Start Camera"), "primary", "camera");
    m_freeze = btn(tr("Freeze"), "outline", "pause");
    m_refresh = btn("", "square", "history");
    m_refresh->setToolTip(tr("Refresh camera list"));
    bl->addWidget(m_stream);
    bl->addWidget(m_freeze);
    bl->addWidget(m_refresh);
    ll->addWidget(bar);

    auto *options = hrow(8);
    m_resolutionBox = new QComboBox;
    m_resolutionBox->addItem("640 x 480", QSize(640, 480));
    m_resolutionBox->addItem("1280 x 720", QSize(1280, 720));
    m_resolutionBox->addItem("1920 x 1080", QSize(1920, 1080));
    m_resolutionBox->setCurrentIndex(1);
    m_resolutionBox->setToolTip(tr("Requested resolution; the camera may choose its nearest supported size."));
    m_mirror = new QCheckBox(tr("Mirror"));
    m_focus = btn(tr("Autofocus"), "soft", "target");
    m_snapshot = btn(tr("Snapshot"), "soft", "download");
    options->addWidget(m_resolutionBox);
    options->addWidget(m_mirror);
    options->addStretch();
    options->addWidget(m_focus);
    options->addWidget(m_snapshot);
    ll->addLayout(options);

    m_camera = new CameraView;
    ll->addWidget(m_camera, 1);

    auto *hint = panel();
    auto *hl = hbox(hint, 16, 12, 16, 12, 12);
    hl->addWidget(iconLabel("scan", QColor("#0B4BCB"), 20));
    m_camHint = T(tr("Press \"Start Camera\" to open the selected webcam."), 14, 400, "#1F2A44");
    m_camHint->setWordWrap(true);
    hl->addWidget(m_camHint, 1);
    hl->addWidget(iconLabel("target", QColor("#737686"), 18));
    hl->addWidget(T("Keep bars\nwell lit", 11, 400, "#737686", true));
    ll->addWidget(hint);
    body->addWidget(left, 1, Qt::AlignTop);

    // ---- right column
    auto *right = vcol(16);
    auto *rc = card();
    rc->setFixedWidth(320);
    auto *rl = vbox(rc, 20, 20, 20, 20, 14);

    m_cam.detectedWording = true;
    m_cam.okKind = "green";
    m_cam.badge = pill("\xE2\x97\x8F Barcode Detected", "green");
    m_cam.badge->setContentsMargins(7, 3, 7, 3);
    { QFont bf = m_cam.badge->font(); bf.setPixelSize(11); m_cam.badge->setFont(bf); }
    rl->addWidget(resultHeader(tr("Decoded Code"), m_cam.badge, 16));
    rl->addWidget(hline());

    auto *pay = panel("panelB");
    auto *pv = vbox(pay, 14, 18, 14, 16, 8);
    auto *cap = T("PAYLOAD CONTENT", 11, 600, "#737686", true);
    cap->setAlignment(Qt::AlignCenter);
    pv->addWidget(cap);
    m_cam.code = bigCode("------", 40, "#0B1C30");
    pv->addWidget(m_cam.code);
    auto *ckw = new QWidget;
    auto *ck = hbox(ckw, 0, 0, 0, 0, 6);
    ck->addStretch();
    ck->addWidget(iconLabel("check", QColor("#0B7A4B"), 14));
    ck->addWidget(T("Mod-10 Check Digit Passed", 11, 400, "#0B7A4B", true));
    ck->addStretch();
    pv->addWidget(ckw);
    m_okOnly << ckw;
    rl->addWidget(pay);

    m_cam.copy = btn(tr("Copy Code"), "deep", "copy");
    m_cam.copy->setMinimumHeight(46);
    wireCopy(m_cam.copy, m_cam.code);
    rl->addWidget(m_cam.copy);
    rl->addWidget(hline());

    m_cam.bounds = T("\xE2\x80\x94", 12, 400, "#0B1C30", true);
    m_cam.stamp = T("\xE2\x80\x94", 12, 400, "#0B1C30", true);
    m_cam.latency = T("\xE2\x80\x94", 12, 600, "#0B7A4B", true);
    rl->addLayout(kv("Symbology", chip("EAN-8", "blue")));
    rl->addLayout(kv("Payload Length", T("6 Digits (Numeric)", 12, 400, "#0B1C30", true)));
    rl->addLayout(kv("Bounding Dimensions", m_cam.bounds));
    rl->addLayout(kv("Scan Timestamp", m_cam.stamp));
    rl->addLayout(kv("Decode Latency", m_cam.latency));

    auto *save = btn(tr("View Scan History"), "tint", "history");
    save->setMinimumHeight(44);
    rl->addWidget(save);
    auto *two = hrow(10);
    auto *re = btn(tr("Re-encode"), "soft", "barcode");
    auto *api = btn(tr("Export Scans"), "soft", "download");
    for (auto *b : {re, api}) b->setStyleSheet("font-size:13px; padding:0 8px;");
    two->addWidget(re, 1);
    two->addWidget(api, 1);
    rl->addLayout(two);
    right->addWidget(rc);

    auto *gs = card();
    gs->setFixedWidth(320);
    auto *gl = hbox(gs, 16, 14, 16, 14, 12);
    gl->addWidget(iconTile("shield", QColor("#0B4BCB"), 42, 22));
    auto *gt = vcol(1);
    gt->addWidget(T("EAN-8 Check Digit", 14, 600, "#0B1C30"));
    gt->addWidget(T("GS1 Mod-10 verified on every decode", 13, 400, "#434655"));
    gl->addLayout(gt, 1);
    right->addWidget(gs);
    right->addStretch();
    body->addLayout(right);
    root->addLayout(body, 1);

    // ---- wiring
    m_freeze->setEnabled(false);
    m_focus->setEnabled(false);
    m_snapshot->setEnabled(false);
    connect(m_refresh, &QPushButton::clicked, this, &DecodePage::cameraRefreshRequested);
    connect(m_focus, &QPushButton::clicked, this, &DecodePage::autofocusRequested);
    connect(save, &QPushButton::clicked, this, [this] { setTab(2); });
    connect(api, &QPushButton::clicked, this, &DecodePage::exportHistory);
    connect(re, &QPushButton::clicked, this, [this] {
        if (isValidCode(m_cam.code->text())) emit generateFromCodeRequested(m_cam.code->text());
    });
    connect(m_snapshot, &QPushButton::clicked, this, [this] {
        const auto frame = m_camera->frame();
        if (frame.isNull()) { setCameraMessage(tr("Wait for the first camera frame.")); return; }
        QString path = QFileDialog::getSaveFileName(this, tr("Save camera snapshot"), "camera.png", tr("PNG image (*.png)"));
        if (path.isEmpty()) return;
        if (!path.endsWith(".png", Qt::CaseInsensitive)) path += ".png";
        if (!frame.save(path, "PNG")) QMessageBox::warning(this, tr("Snapshot"), tr("Could not save the image."));
    });
    connect(m_stream, &QPushButton::clicked, this, [this] {
        if (m_streaming) { stopCamera(); return; }
        if (m_cameraBox->currentData().toInt() < 0) return;
        setCameraActive(true);                       // optimistic; MainWindow reverts it if the camera can't open
        setCameraMessage(tr("Opening camera..."));
        emit cameraStartRequested(m_cameraBox->currentData().toInt());
    });
    connect(m_freeze, &QPushButton::clicked, this, [this] {
        const bool frozen = !m_camera->frozen();
        m_camera->setFrozen(frozen);
        m_freeze->setText(frozen ? " Resume" : " Freeze");
        m_mirror->setEnabled(!frozen);
        setCameraMessage(frozen ? tr("Image and decoded result frozen. Press Resume to continue.") : tr("Hold the barcode steady inside the frame."));
    });
    return page;
}

// ================================================================== screen 4 – upload
QWidget *DecodePage::buildUploadPage()
{
    auto *page = new QWidget;
    page->setObjectName("page");
    auto *root = vbox(page, 24, 22, 24, 24, 18);

    auto *head = hrow(14);
    auto *tt = vcol(4);
    auto *tr1 = hrow(12);
    tr1->addWidget(T(tr("Decode Barcode"), 26, 600, "#0B1C30"));
    tr1->addWidget(pill(tr("Optical Engine Active"), "tint"));
    tr1->addStretch();
    tt->addLayout(tr1);
    tt->addWidget(T(tr("EAN-8 payload extractor & verification stage"), 14, 400, "#434655"));
    head->addLayout(tt);
    head->addStretch();
    head->addWidget(monoPill("<span style='color:#0B4BCB'>\xE2\x97\x8F</span>&nbsp; Decoder Ready"), 0, Qt::AlignTop);
    root->addLayout(head);
    root->addWidget(makeTabs(1), 0, Qt::AlignLeft);

    auto *body = hrow(20);

    // ---- left: input stream
    auto *left = card();
    auto *ll = vbox(left, 24, 22, 24, 24, 14);
    auto *lh = hrow(10);
    lh->addWidget(iconLabel("image", QColor("#0B4BCB"), 22));
    lh->addWidget(T(tr("Image Input Stream"), 18, 600, "#0B1C30"));
    lh->addStretch();
    lh->addWidget(T("Stage: Binary Normalization", 11, 400, "#737686", true));
    ll->addLayout(lh);

    auto *drop = new DropZone;
    ll->addWidget(drop);

    m_loaded = panel();
    auto *ld = vbox(m_loaded, 16, 14, 16, 16, 10);
    auto *lt = hrow();
    lt->addWidget(T("LOADED BUFFER", 11, 600, "#737686", true));
    lt->addStretch();
    auto *remove = btn(tr("Remove"), "danger", "trash");
    lt->addWidget(remove);
    ld->addLayout(lt);

    auto *row = new QFrame;
    row->setObjectName("white");
    auto *rw = hbox(row, 10, 10, 12, 10, 14);
    m_thumb = new QLabel;
    m_thumb->setFixedSize(66, 56);
    m_thumb->setStyleSheet("background:#EFF4FF; border:1px solid #D6E1F7; border-radius:6px;");
    m_thumb->setAlignment(Qt::AlignCenter);
    m_thumb->setPixmap(miniBarPixmap("749205", QSize(48, 40)));
    rw->addWidget(m_thumb);
    auto *info = vcol(3);
    m_fileName = T("sample_barcode_scan.png", 14, 500, "#0B1C30");
    m_fileMeta = new QLabel("142 KB &nbsp;\xE2\x80\xA2&nbsp; <span style='color:#0B4BCB'>Pre-processed 300 DPI</span>");
    m_fileMeta->setStyleSheet("font-size:11px; color:#434655; font-family:" + monoFamily() + ";");
    info->addWidget(m_fileName);
    info->addWidget(m_fileMeta);
    rw->addLayout(info, 1);
    auto *decode = btn(tr("Decode Image"), "deep", "scan");
    decode->setMinimumHeight(42);
    rw->addWidget(decode);
    ld->addWidget(row);
    ll->addWidget(m_loaded);

    ll->addSpacing(4);
    ll->addWidget(T("QUICK-LOAD BENCHMARKS", 11, 600, "#737686", true));
    auto *qr = vcol(8);
    auto *s1 = new QPushButton(tr(" Load Sample 1 (EAN-8: 749205)"));
    auto *s2 = new QPushButton(tr(" Load Sample 2 (Damaged Scan)"));
    for (auto *b : {s1, s2}) {
        b->setObjectName("chipBtn");
        b->setCursor(Qt::PointingHandCursor);
        b->setIcon(makeIcon("barcode", QColor("#0B4BCB"), 16));
        auto *w = new QHBoxLayout;
        w->setContentsMargins(0, 0, 0, 0);
        w->addWidget(b);
        w->addStretch();
        qr->addLayout(w);
    }
    ll->addLayout(qr);
    body->addWidget(left, 1, Qt::AlignTop);

    // ---- right column
    auto *right = vcol(16);
    auto *rc = card();
    rc->setFixedWidth(404);
    auto *rl = vbox(rc, 22, 22, 22, 22, 14);
    m_up.badge = pill("\xE2\x97\x8F  Success", "tint");
    rl->addWidget(resultHeader(tr("Decoded Code"), m_up.badge));

    auto *pay = panel();
    auto *pv = vbox(pay, 14, 18, 14, 16, 10);
    auto *cap = T("PAYLOAD OUTPUT", 11, 600, "#737686", true);
    cap->setAlignment(Qt::AlignCenter);
    pv->addWidget(cap);
    m_up.code = bigCode("------", 48, "#0B1C30");
    pv->addWidget(m_up.code);
    auto *chips = hrow(8);
    chips->addStretch();
    chips->addWidget(chip("EAN-8", "blue"));
    chips->addWidget(chip("1D Linear", "white"));
    chips->addStretch();
    pv->addLayout(chips);
    rl->addWidget(pay);

    auto *ab = hrow(10);
    m_up.copy = btn(tr("Copy Code"), "deep", "copy");
    m_up.copy->setMinimumHeight(44);
    wireCopy(m_up.copy, m_up.code);
    auto *exp = btn(tr("Export Data"), "soft", "download");
    exp->setMinimumHeight(44);
    ab->addWidget(m_up.copy, 1);
    ab->addWidget(exp, 1);
    rl->addLayout(ab);

    m_up.fail = makeFailBox();
    m_up.fail->hide();
    rl->addWidget(m_up.fail);

    rl->addWidget(T("PAYLOAD METADATA", 11, 600, "#737686", true));
    auto *meta = panel();
    auto *mv = vbox(meta, 16, 14, 16, 14, 12);
    mv->addLayout(kv("Code Length", T("6 digits", 13, 700, "#0B1C30", true)));
    auto *valid = new QWidget;
    auto *vh = hbox(valid, 0, 0, 0, 0, 6);
    vh->addWidget(iconLabel("check", QColor("#0B4BCB"), 15));
    vh->addWidget(T("Valid (Mod 10)", 12, 500, "#0B4BCB", true));
    m_okOnly << valid;
    mv->addLayout(kv("Checksum Verification", valid));
    m_up.latency = T("\xE2\x80\x94", 13, 700, "#0B1C30", true);
    m_up.payload = T("\xE2\x80\x94", 13, 700, "#0B1C30", true);
    m_up.bounds = T("\xE2\x80\x94", 13, 700, "#0B1C30", true);
    mv->addLayout(kv("Raster Processing Latency", m_up.latency));
    mv->addLayout(kv("Barcode Size", m_up.bounds));
    mv->addLayout(kv("EAN-8 Payload", m_up.payload));
    rl->addWidget(meta);
    right->addWidget(rc);

    auto *dg = card();
    dg->setFixedWidth(404);
    auto *dl = hbox(dg, 18, 14, 18, 14, 10);
    dl->addWidget(iconLabel("bug", QColor("#737686"), 18));
    dl->addWidget(T("Diagnostics: Preview Error State", 13, 400, "#434655"), 1);
    m_simulate = btn(tr("Simulate Error"), "soft");
    m_simulate->setStyleSheet("font-family:" + monoFamily() + "; font-size:11px; min-height:30px; padding:0 10px;");
    dl->addWidget(m_simulate);
    right->addWidget(dg);
    right->addStretch();
    body->addLayout(right);
    root->addLayout(body, 1);

    // ---- wiring
    connect(drop, &DropZone::fileChosen, this, &DecodePage::loadImage);
    connect(remove, &QPushButton::clicked, this, [this] { m_loaded->hide(); m_path.clear(); resetResult(); });
    connect(decode, &QPushButton::clicked, this, [this] {
        if (m_path.isEmpty()) {
            showFailure(tr("No image loaded. Please upload an image first."), "ERR_NO_IMAGE");
            return;
        }
        emit decodeImageRequested(m_path);
    });
    connect(s1, &QPushButton::clicked, this, [this] { emit sampleRequested("749205", false); });
    connect(s2, &QPushButton::clicked, this, [this] { emit sampleRequested("749205", true); });
    connect(exp, &QPushButton::clicked, this, &DecodePage::exportHistory);
    connect(m_simulate, &QPushButton::clicked, this, [this] {
        m_simulating = !m_simulating;
        if (m_simulating) { showFailure(); m_simulate->setText(tr("Restore")); }
        else { resetResult(); m_simulate->setText(tr("Simulate Error")); }
    });
    return page;
}

void DecodePage::loadImage(const QString &path)
{
    const QFileInfo fi(path);
    if (!fi.exists() || !fi.isFile()) {
        m_path.clear(); m_loaded->hide();
        showFailure(tr("File not found."), "ERR_FILE_NOT_FOUND");
        return;
    }
    if (fi.size() > 32LL * 1024 * 1024) {
        m_path.clear(); m_loaded->hide();
        showFailure(tr("File exceeds the 32 MB limit."), "ERR_FILE_TOO_LARGE");
        return;
    }
    QPixmap pm(path);
    if (pm.isNull()) {
        m_path.clear(); m_loaded->hide();
        showFailure(tr("Unsupported or corrupted image."), "ERR_BAD_IMAGE");
        return;
    }
    m_path = path;
    m_thumb->setPixmap(pm.scaled(QSize(54, 44), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    m_fileName->setText(fi.fileName());
    m_fileMeta->setText(QString("%1 KB &nbsp;\xE2\x80\xA2&nbsp; <span style='color:#0B4BCB'>%2 \xC3\x97 %3 px</span>")
                            .arg(qMax<qint64>(1, fi.size() / 1024)).arg(pm.width()).arg(pm.height()));
    m_loaded->show();
    resetResult();
}

// ================================================================== screen 3 – scan logs overview
QWidget *DecodePage::buildLogsPage()
{
    auto *page = new QWidget;
    page->setObjectName("page");
    auto *root = vbox(page, 24, 22, 24, 24, 18);

    // header card
    auto *hc = card();
    auto *hh = hbox(hc, 22, 18, 22, 18, 12);
    auto *tt = vcol(6);
    auto *tr1 = hrow(12);
    tr1->addWidget(T(tr("Decode Barcode"), 24, 600, "#0B1C30"));
    tr1->addWidget(chip("Increment 1", "blue"));
    auto *ready = new QLabel("\xE2\x97\x8F  Ready to Ingest");
    ready->setStyleSheet("background:#E8EFFD; color:#434655; border-radius:6px; padding:2px 8px; font-size:11px; font-family:" + monoFamily() + ";");
    tr1->addWidget(ready);
    tr1->addStretch();
    tt->addLayout(tr1);
    auto *logsSub = T(tr("High-throughput automated optical decoding & verification telemetry."), 14, 400, "#434655");
    logsSub->setWordWrap(true);
    tt->addWidget(logsSub);
    hh->addLayout(tt, 1);
    hh->addWidget(makeTabs(2), 0, Qt::AlignVCenter);
    root->addWidget(hc);

    auto *body = hrow(20);

    // ================= left column
    auto *left = vcol(18);

    auto *two = hrow(18);
    auto makeEntry = [&](const QString &icon, const QString &meta, const QString &title, const QString &desc) {
        auto *c = card();
        auto *v = vbox(c, 18, 18, 18, 18, 10);
        auto *top = hrow();
        top->addWidget(iconTile(icon, QColor("#0B4BCB"), 44, 22));
        top->addStretch();
        top->addWidget(T(meta, 11, 400, "#737686", true), 0, Qt::AlignTop);
        v->addLayout(top);
        v->addSpacing(4);
        v->addWidget(T(title, 18, 600, "#0B1C30"));
        auto *d = T(desc, 14, 400, "#434655");
        d->setWordWrap(true);
        v->addWidget(d);
        return std::pair<QFrame *, QVBoxLayout *>(c, v);
    };

    auto cam = makeEntry("camera", "Live capture", tr("Camera Scanner"),
                         tr("Real-time continuous decoding via direct hardware video stream & industrial optics."));
    cam.second->addStretch();
    auto *dev = panel();
    auto *dv = vbox(dev, 12, 10, 12, 10, 8);
    auto *dr = hrow();
    dr->addWidget(T("Select camera in scanner", 12, 500, "#0B1C30", true));
    dr->addStretch();
    dr->addWidget(T("USB / built-in", 12, 400, "#737686", true));
    dv->addLayout(dr);
    auto *openCam = btn(tr("Open Camera Scanner"), "deep", "scan");
    openCam->setMinimumHeight(40);
    dv->addWidget(openCam);
    cam.second->addWidget(dev);
    two->addWidget(cam.first, 1);

    auto up = makeEntry("upload", "Max: 32MB", tr("File Upload"),
                        tr("Decode PNG, JPG, or BMP barcode images."));
    up.second->addStretch();
    auto *dz = panel();
    auto *dzl = vbox(dz, 12, 10, 12, 10, 8);
    auto *dzt = hrow(8);
    dzt->addStretch();
    dzt->addWidget(iconLabel("image", QColor("#0B4BCB"), 15));
    dzt->addWidget(T("Choose an image to decode", 12, 500, "#0B4BCB", true));
    dzt->addStretch();
    dzl->addLayout(dzt);
    auto *openUp = btn(tr("Upload Barcode Image"), "outline", "folder");
    openUp->setMinimumHeight(40);
    dzl->addWidget(openUp);
    up.second->addWidget(dz);
    two->addWidget(up.first, 1);
    left->addLayout(two);

    // history table
    auto *hist = card();
    auto *hv = vbox(hist, 22, 20, 22, 20, 12);
    auto *hr = hrow(10);
    hr->addWidget(iconLabel("history", QColor("#0B4BCB"), 22));
    hr->addWidget(T(tr("Recent Scans & Session History"), 18, 600, "#0B1C30"));
    hr->addStretch();
    auto *clearHistory = btn(tr("Clear History"), "link", "trash");
    hr->addWidget(clearHistory);
    hv->addLayout(hr);

    m_history = new QTableWidget(0, 5);
    m_history->setObjectName("scanHistory");
    m_history->setHorizontalHeaderLabels({tr("Timestamp"), tr("Format"), tr("Code"), tr("Payload"), tr("Latency")});
    m_history->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_history->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_history->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    m_history->horizontalHeader()->setStretchLastSection(true);
    m_history->verticalHeader()->hide();
    m_history->setMinimumHeight(230);
    hv->addWidget(m_history);
    hv->addWidget(T(tr("Up to 1,000 scans this session. Export CSV to keep them."), 12, 400, "#737686"));
    left->addWidget(hist);
    left->addStretch();
    body->addLayout(left, 1);

    // ================= right column
    auto *right = vcol(16);
    auto *rc = card();
    rc->setFixedWidth(386);
    auto *rl = vbox(rc, 22, 22, 22, 22, 14);
    m_log.badge = pill("\xE2\x97\x8F  Success", "tint");
    rl->addWidget(resultHeader(tr("Decoded Code"), m_log.badge));

    auto *pay = panel();
    auto *pv = vbox(pay, 18, 16, 18, 14, 10);
    auto *pr = hrow();
    pr->addWidget(T("PAYLOAD STRING", 11, 600, "#737686", true));
    pr->addStretch();
    pr->addWidget(T("6 Digits Numerical", 11, 400, "#0B4BCB", true));
    pv->addLayout(pr);
    auto *pr2 = hrow(10);
    m_log.code = T("------", 32, 700, "#0B1C30", true);
    pr2->addWidget(m_log.code);
    pr2->addStretch();
    m_log.copy = btn(tr("Copy Code"), "deep", "copy");
    wireCopy(m_log.copy, m_log.code);
    pr2->addWidget(m_log.copy);
    pv->addLayout(pr2);
    rl->addWidget(pay);

    auto *tiles = new QGridLayout;
    tiles->setHorizontalSpacing(10);
    tiles->setVerticalSpacing(10);
    struct Tile { const char *k, *v, *s; bool blue; };
    const Tile tl[] = {{"Format Symbology", "EAN-8", "Auto-detected profile", false},
                       {"Decode Latency", "\xE2\x80\x94", "Measured on the last decode", true},
                       {"Check Digit", "Mod-10", "Verified on every decode", false},
                       {"Timestamp", "\xE2\x80\x94", "Time of the last decode", false}};
    for (int i = 0; i < 4; ++i) {
        auto *t = panel();
        auto *tv = vbox(t, 14, 12, 14, 12, 3);
        tv->addWidget(T(tl[i].k, 12, 500, "#434655", true));
        auto *valLbl = T(tl[i].v, 16, 600, "#0B1C30");
        if (i == 1) m_log.latency = valLbl;
        if (i == 3) m_log.stamp = valLbl;
        tv->addWidget(valLbl);
        auto *s = T(tl[i].s, 11, 400, tl[i].blue ? "#0B4BCB" : "#737686", true);
        s->setWordWrap(true);
        tv->addWidget(s);
        tv->addStretch();
        tiles->addWidget(t, i / 2, i % 2);
    }
    rl->addLayout(tiles);

    auto *acts = hrow(10);
    auto *gen = btn(tr("Generate from\nthis code"), "tint", "barcode");
    gen->setMinimumHeight(48);
    auto *exl = btn(tr("Export Scan Log"), "soft", "download");
    exl->setMinimumHeight(48);
    acts->addWidget(gen, 1);
    acts->addWidget(exl, 1);
    rl->addLayout(acts);
    right->addWidget(rc);

    m_log.fail = makeFailBox();
    m_log.fail->hide();
    m_log.fail->setFixedWidth(386);
    right->addWidget(m_log.fail);

    auto *clip = card();
    clip->setFixedWidth(386);
    auto *cl = hbox(clip, 16, 14, 16, 14, 12);
    cl->addWidget(iconTile("bolt", QColor("#0B4BCB"), 38, 20));
    auto *ct = vcol(1);
    ct->addWidget(T("Automatic Clipboard Passthrough", 14, 600, "#0B1C30"));
    auto *cs = T("Push successful decodes straight to system clipboard", 13, 400, "#434655");
    cs->setWordWrap(true);
    ct->addWidget(cs);
    cl->addLayout(ct, 1);
    auto *autoCopy = new ToggleSwitch(false);
    cl->addWidget(autoCopy);
    connect(autoCopy, &ToggleSwitch::toggled, this, [this](bool on) { m_autoCopy = on; });
    right->addWidget(clip);
    right->addStretch();
    body->addLayout(right);
    root->addLayout(body, 1);

    // ---- wiring
    connect(openCam, &QPushButton::clicked, this, [this] { setTab(0); });
    connect(openUp, &QPushButton::clicked, this, [this] { setTab(1); });
    connect(exl, &QPushButton::clicked, this, &DecodePage::exportHistory);
    connect(clearHistory, &QPushButton::clicked, this, [this] { m_history->setRowCount(0); });
    connect(gen, &QPushButton::clicked, this, [this] {
        if (isValidCode(m_log.code->text())) emit generateFromCodeRequested(m_log.code->text());
    });
    return page;
}

// ================================================================== result state (all screens)
static const QString kDash = QString::fromUtf8("\xE2\x80\x94");

void DecodePage::applyResult(const ResultUi &r, const QString &code)
{
    if (!r.code) return;
    r.code->setText(code);
    r.code->setStyleSheet("color:#0B1C30;");
    if (r.badge) setBadge(r.badge, r.detectedWording ? "\xE2\x97\x8F Barcode Detected" : "\xE2\x97\x8F  Success", r.okKind);
    if (r.copy) r.copy->setEnabled(true);
    if (r.fail) r.fail->hide();
}

void DecodePage::clearDetails(const ResultUi &r)
{
    for (QLabel *l : {r.bounds, r.stamp, r.latency, r.payload})
        if (l) l->setText(kDash);
}

void DecodePage::showResult(const QString &code, const ScanInfo &info)
{
    if (!isValidCode(code)) {
        showFailure(tr("Invalid code: must be exactly 6 digits, first digit non-zero."), "ERR_INVALID_CODE");
        return;
    }
    applyResult(m_cam, code);
    applyResult(m_up, code);
    applyResult(m_log, code);
    m_log.copy->setEnabled(true);
    if (m_log.fail) m_log.fail->hide();
    for (QWidget *w : m_okOnly) w->show();

    const QString stamp = QTime::currentTime().toString("HH:mm:ss.zzz");
    const QString latency = info.latencyMs >= 0 ? QString("%1 ms").arg(info.latencyMs) : kDash;
    const QString box = info.box.isEmpty() ? kDash : QString("%1 \xC3\x97 %2 px").arg(info.box.width()).arg(info.box.height());
    const QString payload = info.payload.isEmpty() ? kDash : info.payload;
    if (m_cam.bounds) m_cam.bounds->setText(box);
    if (m_cam.stamp) m_cam.stamp->setText(stamp);
    if (m_cam.latency) m_cam.latency->setText(latency);
    if (m_up.bounds) m_up.bounds->setText(box);
    if (m_up.latency) m_up.latency->setText(latency);
    if (m_up.payload) m_up.payload->setText(payload);
    if (m_log.latency) m_log.latency->setText(latency);
    if (m_log.stamp) m_log.stamp->setText(stamp);
    if (m_autoCopy) QApplication::clipboard()->setText(code);
    m_history->insertRow(0);
    const QStringList values{QDateTime::currentDateTime().toString(Qt::ISODateWithMs), "EAN-8", code, payload, latency};
    for (int column = 0; column < values.size(); ++column)
        m_history->setItem(0, column, new QTableWidgetItem(values[column]));
    if (m_history->rowCount() > 1000) m_history->removeRow(1000);
}

void DecodePage::showFailure(const QString &reason, const QString &errCode)
{
    for (const ResultUi *r : {&m_cam, &m_up, &m_log}) {
        if (!r->code) continue;
        r->code->setText("------");
        r->code->setStyleSheet("color:#C2CCDF;");
        if (r->badge) setBadge(r->badge, "Could not read barcode", "red");
        if (r->copy) r->copy->setEnabled(false);
        clearDetails(*r);
    }
    for (QWidget *w : m_okOnly) w->hide();
    for (QFrame *f : {m_up.fail, m_log.fail}) {
        if (!f) continue;
        if (auto *t = f->findChild<QLabel *>("failReason"))
            t->setText(reason.isEmpty() ? tr("Could not decode the barcode.") : reason);
        if (auto *c = f->findChild<QLabel *>("failCode"))
            c->setText(errCode.isEmpty() ? QString("ERR_UNKNOWN") : errCode);
        f->show();
    }
}

void DecodePage::resetResult()
{
    for (const ResultUi *r : {&m_cam, &m_up, &m_log}) {
        if (!r->code) continue;
        r->code->setText("------");
        r->code->setStyleSheet("color:#C2CCDF;");
        if (r->badge) setBadge(r->badge, "\xE2\x97\x8F  Waiting", "idle");
        if (r->copy) r->copy->setEnabled(false);
        if (r->fail) r->fail->hide();
        clearDetails(*r);
    }
    for (QWidget *w : m_okOnly) w->hide();
    if (m_log.fail) m_log.fail->hide();
    if (m_log.code) m_log.code->setText("------");
    if (m_log.copy) m_log.copy->setEnabled(false);
    if (m_log.latency) m_log.latency->setText(kDash);
    if (m_log.stamp) m_log.stamp->setText(kDash);
}

void DecodePage::setCameraActive(bool on)
{
    m_streaming = on;
    m_camera->setRunning(on);
    m_stream->setText(on ? " Stop Camera" : " Start Camera");
    setKind(m_stream, on ? "deep" : "primary");
    m_cameraBox->setEnabled(!on);
    m_resolutionBox->setEnabled(!on);
    m_refresh->setEnabled(!on);
    m_focus->setEnabled(on);
    m_snapshot->setEnabled(on);
    m_mirror->setEnabled(true);
    m_engine->setText(on ? "ENGINE: Camera active" : "ENGINE: Idle");
    m_freeze->setEnabled(on);
    m_freeze->setText(" Freeze");
    if (!on) setCameraMessage(tr("Press \"Start Camera\" to open the selected webcam."));
    else     setCameraMessage(tr("Hold the barcode steady inside the frame."));
}

void DecodePage::setCameraMessage(const QString &text, bool error)
{
    m_camHint->setText(text);
    m_camHint->setStyleSheet(QString("font-size:14px; color:%1;").arg(error ? "#BA1A1A" : "#1F2A44"));
}

void DecodePage::stopCamera()
{
    if (!m_streaming) return;
    setCameraActive(false);
    emit cameraStopRequested();
}

void DecodePage::setCameraList(const QStringList &names)
{
    m_cameraBox->clear();
    for (int i = 0; i < names.size(); ++i) m_cameraBox->addItem(names[i], i);
    if (names.isEmpty()) m_cameraBox->addItem(tr("No camera found"), -1);
    m_stream->setEnabled(!names.isEmpty());
    if (names.isEmpty()) setCameraMessage(tr("No camera detected. Connect a webcam, then refresh the camera list."));
    else setCameraMessage(tr("Press \"Start Camera\" to open the selected webcam."));
}

QSize DecodePage::cameraResolution() const { return m_resolutionBox->currentData().toSize(); }
bool DecodePage::mirrorCamera() const { return m_mirror->isChecked(); }

void DecodePage::exportHistory()
{
    if (!m_history->rowCount()) {
        QMessageBox::information(this, tr("Export scans"), tr("No successful scans to export yet."));
        return;
    }
    QString path = QFileDialog::getSaveFileName(this, tr("Export scans"), "scans.csv", tr("CSV (*.csv)"));
    if (path.isEmpty()) return;
    if (!path.endsWith(".csv", Qt::CaseInsensitive)) path += ".csv";
    QSaveFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, tr("Export scans"), file.errorString());
        return;
    }
    QTextStream out(&file);
    out << "Timestamp,Symbology,Code,Payload,Latency\n";
    for (int row = 0; row < m_history->rowCount(); ++row) {
        for (int column = 0; column < m_history->columnCount(); ++column) {
            QString value = m_history->item(row, column)->text();
            value.replace('"', "\"\"");
            if (column) out << ',';
            out << '"' << value << '"';
        }
        out << '\n';
    }
    out.flush();
    if (out.status() != QTextStream::Ok || !file.commit())
        QMessageBox::warning(this, tr("Export scans"), tr("Could not write the scan log."));
}
