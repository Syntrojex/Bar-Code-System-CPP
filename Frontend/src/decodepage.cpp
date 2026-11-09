#include "decodepage.h"
#include "icons.h"
#include "widgets.h"

#include <QApplication>
#include <QClipboard>
#include <QComboBox>
#include <QFileInfo>
#include <QGridLayout> // used by the stat/tile grids
#include <QPainter>
#include <QProgressBar>
#include <QRegularExpression>
#include <QStackedWidget>
#include <QTime>
#include <QTimer>

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
    QObject::connect(b, &QPushButton::clicked, b, [b, code]
                     {
        if (b->property("copied").toBool()) return;     // double-click guard
        QApplication::clipboard()->setText(code->text());
        const QString old = b->text();
        b->setProperty("copied", true);
        b->setText(" Copied!");
        QTimer::singleShot(1300, b, [b, old] {
            b->setText(old);
            b->setProperty("copied", false);
        }); });
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
}

void DecodePage::setTab(int i)
{
    if (i != 0) stopCamera();                        // never keep the webcam on while another tab is shown
    m_stack->setCurrentIndex(i);
}

SegBar *DecodePage::makeTabs(int current)
{
    auto *bar = new SegBar({tr("Scan with Camera"), tr("Upload Image"), tr("Scan Logs")},
                           {"camera", "upload", "history"}, current);
    connect(bar, &SegBar::currentChanged, this, &DecodePage::setTab);
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
    head->addWidget(monoPill("<span style='color:#0B4BCB'>\xE2\x97\x8F</span>&nbsp; ENGINE: <b>Idle</b>"), 0, Qt::AlignTop);
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
    m_cameraBox->addItems({tr("Camera 0 (default)"), tr("Camera 1"), tr("Camera 2")});
    m_cameraBox->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLengthWithIcon);
    m_cameraBox->setMinimumContentsLength(10);
    m_cameraBox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    bl->addWidget(m_cameraBox, 1);
    m_stream = btn(tr("Start Camera"), "primary", "camera");
    m_freeze = btn(tr("Freeze"), "outline", "pause");
    auto *cfg = btn("", "square", "sliders");
    bl->addWidget(m_stream);
    bl->addWidget(m_freeze);
    bl->addWidget(cfg);
    ll->addWidget(bar);

    m_camera = new CameraView;
    ll->addWidget(m_camera, 1);

    auto *hint = panel();
    auto *hl = hbox(hint, 16, 12, 16, 12, 12);
    hl->addWidget(iconLabel("scan", QColor("#0B4BCB"), 20));
    m_camHint = T(tr("Press \"Start Camera\" to open the selected webcam."), 14, 400, "#1F2A44");
    m_camHint->setWordWrap(true);
    hl->addWidget(m_camHint, 1);
    hl->addWidget(iconLabel("target", QColor("#737686"), 18));
    hl->addWidget(T("Tap to\nRefocus", 11, 400, "#737686", true));
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
    {
        QFont bf = m_cam.badge->font();
        bf.setPixelSize(11);
        m_cam.badge->setFont(bf);
    }
    rl->addWidget(resultHeader(tr("Decoded Code"), m_cam.badge, 16));
    rl->addWidget(hline());

    auto *pay = panel("panelB");
    auto *pv = vbox(pay, 14, 18, 14, 16, 8);
    auto *cap = T("PAYLOAD CONTENT", 11, 600, "#737686", true);
    cap->setAlignment(Qt::AlignCenter);
    pv->addWidget(cap);
    m_cam.code = bigCode("492817", 40, "#0B1C30");
    pv->addWidget(m_cam.code);
    auto *ck = hrow(6);
    ck->addStretch();
    ck->addWidget(iconLabel("check", QColor("#0B7A4B"), 14));
    ck->addWidget(T("Modulo-103 Checksum Passed", 11, 400, "#0B7A4B", true));
    ck->addStretch();
    pv->addLayout(ck);
    rl->addWidget(pay);

    m_cam.copy = btn(tr("Copy Code"), "deep", "copy");
    m_cam.copy->setMinimumHeight(46);
    wireCopy(m_cam.copy, m_cam.code);
    rl->addWidget(m_cam.copy);
    rl->addWidget(hline());

    rl->addLayout(kv("Symbology", chip("Code 128 (Auto B)", "blue")));
    rl->addLayout(kv("Payload Length", T("6 Digits (Numeric)", 12, 400, "#0B1C30", true)));
    rl->addLayout(kv("Bounding Dimensions", T("320 \xC3\x97 94 px", 12, 400, "#0B1C30", true)));
    rl->addLayout(kv("Scan Timestamp", T("07:25:44.536 UTC", 12, 400, "#0B1C30", true)));
    rl->addLayout(kv("Decode Latency", T("14.2 ms", 12, 600, "#0B7A4B", true)));

    auto *save = btn(tr("Save to Scan History"), "tint", "history");
    save->setMinimumHeight(44);
    rl->addWidget(save);
    auto *two = hrow(10);
    auto *re = btn(tr("Re-encode"), "soft", "barcode");
    auto *api = btn(tr("Dispatch API"), "soft", "arrow");
    for (auto *b : {re, api})
        b->setStyleSheet("font-size:13px; padding:0 8px;");
    two->addWidget(re, 1);
    two->addWidget(api, 1);
    rl->addLayout(two);
    right->addWidget(rc);

    auto *gs = card();
    gs->setFixedWidth(320);
    auto *gl = hbox(gs, 16, 14, 16, 14, 12);
    gl->addWidget(iconTile("shield", QColor("#0B4BCB"), 42, 22));
    auto *gt = vcol(1);
    gt->addWidget(T("GS1 Compliance Check", 14, 600, "#0B1C30"));
    gt->addWidget(T("Standard ISO/IEC 15417 Verified", 13, 400, "#434655"));
    gl->addLayout(gt, 1);
    right->addWidget(gs);
    right->addStretch();
    body->addLayout(right);
    root->addLayout(body, 1);

    // ---- wiring
<<<<<<< HEAD
    m_freeze->setEnabled(false);
    connect(m_stream, &QPushButton::clicked, this, [this] {
        if (m_streaming) { stopCamera(); return; }
        setCameraActive(true);                       // optimistic; MainWindow reverts it if the camera can't open
        setCameraMessage(tr("Opening camera..."));
        emit cameraStartRequested(m_cameraBox->currentIndex());
    });
    connect(m_freeze, &QPushButton::clicked, this, [this] {
        const bool frozen = !m_camera->frozen();
=======
    connect(m_stream, &QPushButton::clicked, this, [this]
            {
        m_streaming = !m_streaming;
        m_camera->setRunning(m_streaming);
        m_stream->setText(m_streaming ? " Stream Active" : " Start Stream");
        setKind(m_stream, m_streaming ? "deep" : "primary");
        if (m_streaming) emit cameraStartRequested(m_cameraBox->currentIndex()); else emit cameraStopRequested(); });
    connect(m_freeze, &QPushButton::clicked, this, [this]
            {
        static bool frozen = false;
        frozen = !frozen;
>>>>>>> 9dbc7fbd625c9785052ddb1dd8ebe4b824bb6694
        m_camera->setFrozen(frozen);
        m_freeze->setText(frozen ? " Resume" : " Freeze"); });
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
    tt->addWidget(T(tr("Multi-format payload extractor & verification stage"), 14, 400, "#434655"));
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
    auto *s1 = new QPushButton(tr(" Load Sample 1 (Code 128: 749205)"));
    auto *s2 = new QPushButton(tr(" Load Sample 2 (Damaged Scan)"));
    for (auto *b : {s1, s2})
    {
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
    m_up.code = bigCode("749205", 48, "#0B1C30");
    pv->addWidget(m_up.code);
    auto *chips = hrow(8);
    chips->addStretch();
    chips->addWidget(chip("CODE_128", "blue"));
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
    vh->addWidget(T("Valid (Mod 103)", 12, 500, "#0B4BCB", true));
    mv->addLayout(kv("Checksum Verification", valid));
    mv->addLayout(kv("Raster Processing Latency", T("18 ms", 13, 700, "#0B1C30", true)));
    mv->addLayout(kv("Confidence Rating", T("99.8%", 13, 700, "#0B1C30", true)));
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
    connect(remove, &QPushButton::clicked, this, [this]
            { m_loaded->hide(); m_path.clear(); resetResult(); });
    connect(decode, &QPushButton::clicked, this, [this]
            {
        if (m_path.isEmpty()) {
            showFailure(tr("No image loaded. Please upload an image first."), "ERR_NO_IMAGE");
            return;
        }
        emit decodeImageRequested(m_path); });
    connect(s1, &QPushButton::clicked, this, [this]
            {
        m_loaded->show(); m_path.clear();
        m_fileName->setText("sample_barcode_scan.png");
        m_thumb->setPixmap(miniBarPixmap("749205", QSize(48, 40)));
        showResult("749205"); });
    connect(s2, &QPushButton::clicked, this, [this]
            {
        m_loaded->show(); m_path.clear();
        m_fileName->setText("sample_damaged_scan.png");
        m_thumb->setPixmap(miniBarPixmap("000000", QSize(48, 40)));
        showFailure(); });
    connect(m_simulate, &QPushButton::clicked, this, [this]
            {
        m_simulating = !m_simulating;
        if (m_simulating) { showFailure(); m_simulate->setText(tr("Restore Success")); }
        else { showResult("749205"); m_simulate->setText(tr("Simulate Error")); } });
    return page;
}

void DecodePage::loadImage(const QString &path)
{
    const QFileInfo fi(path);
    if (!fi.exists() || !fi.isFile())
    {
        m_path.clear();
        m_loaded->hide();
        showFailure(tr("File not found."), "ERR_FILE_NOT_FOUND");
        return;
    }
    if (fi.size() > 32LL * 1024 * 1024)
    {
        m_path.clear();
        m_loaded->hide();
        showFailure(tr("File exceeds the 32 MB limit."), "ERR_FILE_TOO_LARGE");
        return;
    }
    QPixmap pm(path);
    if (pm.isNull())
    {
        m_path.clear();
        m_loaded->hide();
        showFailure(tr("Unsupported or corrupted image."), "ERR_BAD_IMAGE");
        return;
    }
    m_path = path;
    m_thumb->setPixmap(pm.scaled(QSize(54, 44), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    m_fileName->setText(fi.fileName());
    m_fileMeta->setText(QString("%1 KB &nbsp;\xE2\x80\xA2&nbsp; <span style='color:#0B4BCB'>%2 \xC3\x97 %3 px</span>")
                            .arg(qMax<qint64>(1, fi.size() / 1024))
                            .arg(pm.width())
                            .arg(pm.height()));
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
    auto makeEntry = [&](const QString &icon, const QString &meta, const QString &title, const QString &desc)
    {
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

    auto cam = makeEntry("camera", "FPS: 60.0", tr("Camera Scanner"),
                         tr("Real-time continuous decoding via direct hardware video stream & industrial optics."));
    cam.second->addStretch();
    auto *dev = panel();
    auto *dv = vbox(dev, 12, 10, 12, 10, 8);
    auto *dr = hrow();
    dr->addWidget(T("\xE2\x97\x8F  Integrated HD Webcam", 12, 500, "#0B1C30", true));
    dr->addStretch();
    dr->addWidget(T("1080p", 12, 400, "#737686", true));
    dv->addLayout(dr);
    auto *openCam = btn(tr("Open Camera Scanner"), "deep", "scan");
    openCam->setMinimumHeight(40);
    dv->addWidget(openCam);
    cam.second->addWidget(dev);
    two->addWidget(cam.first, 1);

    auto up = makeEntry("upload", "Max: 32MB", tr("File Upload"),
                        tr("Decode high-resolution PNG, JPG, or TIFF raster files with automatic noise removal."));
    up.second->addStretch();
    auto *dz = panel();
    auto *dzl = vbox(dz, 12, 10, 12, 10, 8);
    auto *dzt = hrow(8);
    dzt->addStretch();
    dzt->addWidget(iconLabel("image", QColor("#0B4BCB"), 15));
    dzt->addWidget(T("Drop barcode here", 12, 500, "#0B4BCB", true));
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
    hr->addWidget(btn(tr("Clear History"), "link", "trash"));
    hv->addLayout(hr);

    const char *cols[] = {"Timestamp", "Symbology Format", "Decoded Value", "Engine Metric", "Action"}; // 12px headers
    auto *hdr = panel();
    hdr->setFixedHeight(40);
    auto *hl = hbox(hdr, 14, 0, 14, 0, 10);
    const int stretch[] = {20, 24, 20, 22, 10};
    for (int i = 0; i < 5; ++i)
    {
        auto *c = T(cols[i], 12, 600, "#0B1C30");
        if (i == 4)
            c->setAlignment(Qt::AlignCenter);
        hl->addWidget(c, stretch[i]);
    }
    hv->addWidget(hdr);

    struct Row
    {
        const char *ts, *fmt, *val;
        int ms;
        bool first;
    };
    const Row rows[] = {{"14:28:02.11", "Code 128", "839104", 42, true},
                        {"14:26:45.88", "EAN-8", "552190", 38, false},
                        {"14:20:19.04", "DataMatrix", "901423", 51, false}};
    for (const Row &r : rows)
    {
        auto *rowW = new QWidget;
        rowW->setFixedHeight(58);
        auto *rl = hbox(rowW, 14, 0, 14, 0, 10);
        rl->addWidget(T(r.ts, 11, 400, "#737686", true), stretch[0]);
        auto *fc = new QWidget;
        auto *fh = hbox(fc, 0, 0, 0, 0, 0);
        auto *fch = chip(r.fmt, "blue");
        fh->addWidget(fch, 0, Qt::AlignVCenter);
        fh->addStretch();
        rl->addWidget(fc, stretch[1]);
        rl->addWidget(T(r.val, 14, 700, r.first ? "#0B4BCB" : "#0B1C30", true), stretch[2]);
        auto *mc = new QWidget;
        auto *mh = hbox(mc, 0, 0, 0, 0, 8);
        mh->addWidget(T(QString("%1ms").arg(r.ms), 11, 400, "#737686", true));
        auto *pb = new QProgressBar;
        pb->setTextVisible(false);
        pb->setRange(0, 100);
        pb->setValue(100 - r.ms);
        pb->setFixedWidth(64);
        mh->addWidget(pb);
        mh->addStretch();
        rl->addWidget(mc, stretch[3]);
        auto *cp = btn("", "link", "copy");
        rl->addWidget(cp, stretch[4], Qt::AlignCenter);
        hv->addWidget(rowW);
    }
    left->addWidget(hist);

    auto *stats = hrow(14);
    struct Stat
    {
        const char *k, *v, *s;
    };
    const Stat sts[] = {{"Total Parsed", "1,248", "+12.4% this hr"}, {"Success Rate", "99.78%", "0.02% error margin"}, {"Avg Latency", "41.2ms", "Sub-frame lock"}, {"Format Buffer", "1D & 2D", "All engines active"}};
    for (const Stat &s : sts)
    {
        auto *c = card();
        auto *v = vbox(c, 16, 14, 16, 14, 4);
        v->addWidget(T(s.k, 13, 500, "#434655"));
        v->addWidget(T(s.v, 24, 600, "#0B1C30"));
        auto *sub = T(s.s, 11, 400, QString(s.k) == "Avg Latency" ? "#737686" : "#0B4BCB", true);
        sub->setWordWrap(true);
        v->addWidget(sub);
        c->setMinimumWidth(0);
        stats->addWidget(c, 1);
    }
    left->addLayout(stats);
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
    m_log.code = T("839104", 32, 700, "#0B1C30", true);
    pr2->addWidget(m_log.code);
    pr2->addStretch();
    m_log.copy = btn(tr("Copy Code"), "deep", "copy");
    wireCopy(m_log.copy, m_log.code);
    pr2->addWidget(m_log.copy);
    pv->addLayout(pr2);
    auto *pr3 = hrow();
    pr3->addWidget(new MiniBarcode("839104", QSize(80, 30)));
    pr3->addStretch();
    pr3->addWidget(T("Checksum [Mod103]: OK", 11, 400, "#737686", true));
    pv->addLayout(pr3);
    rl->addWidget(pay);

    auto *tiles = new QGridLayout;
    tiles->setHorizontalSpacing(10);
    tiles->setVerticalSpacing(10);
    struct Tile
    {
        const char *k, *v, *s;
        bool blue;
    };
    const Tile tl[] = {{"Format Symbology", "EAN-8 / Code 128", "Auto-detected profile", false},
                       {"Decode Latency", "42ms", "Ultra-fast hardware tick", true},
                       {"Match Confidence", "99.8%", "High SNR clarity", false},
                       {"Timestamp", "Just now", "14:28:02.11 UTC", false}};
    for (int i = 0; i < 4; ++i)
    {
        auto *t = panel();
        auto *tv = vbox(t, 14, 12, 14, 12, 3);
        tv->addWidget(T(tl[i].k, 12, 500, "#434655", true));
        tv->addWidget(T(tl[i].v, 16, 600, "#0B1C30"));
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
    cl->addWidget(new ToggleSwitch(true));
    right->addWidget(clip);
    right->addStretch();
    body->addLayout(right);
    root->addLayout(body, 1);

    // ---- wiring
    connect(openCam, &QPushButton::clicked, this, [this]
            { setTab(0); });
    connect(openUp, &QPushButton::clicked, this, [this]
            { setTab(1); });
    connect(gen, &QPushButton::clicked, this, [this]
            { emit generateFromCodeRequested(m_log.code->text()); });
    return page;
}

// ================================================================== result state (all screens)
void DecodePage::applyResult(const ResultUi &r, const QString &code)
{
    if (!r.code)
        return;
    r.code->setText(code);
    r.code->setStyleSheet("color:#0B1C30;");
    if (r.badge)
        setBadge(r.badge, r.detectedWording ? "\xE2\x97\x8F Barcode Detected" : "\xE2\x97\x8F  Success", r.okKind);
    if (r.copy)
        r.copy->setEnabled(true);
    if (r.fail)
        r.fail->hide();
}

void DecodePage::showResult(const QString &code)
{
    if (!isValidCode(code))
    {
        showFailure(tr("Invalid code: must be exactly 6 digits, first digit non-zero."), "ERR_INVALID_CODE");
        return;
    }
    applyResult(m_cam, code);
    applyResult(m_up, code);
    m_log.code->setText(code); // overview keeps its permanent telemetry box
    m_log.copy->setEnabled(true);
<<<<<<< HEAD
    if (m_log.fail) m_log.fail->hide();
=======
    if (m_log.fail)
        m_log.fail->hide();
    m_camera->setPreviewCode(code);
>>>>>>> 9dbc7fbd625c9785052ddb1dd8ebe4b824bb6694
}

void DecodePage::showFailure(const QString &reason, const QString &errCode)
{
    for (const ResultUi *r : {&m_cam, &m_up})
    {
        if (!r->code)
            continue;
        r->code->setText("------");
        r->code->setStyleSheet("color:#C2CCDF;");
        if (r->badge)
            setBadge(r->badge, "Could not read barcode", "red");
        if (r->copy)
            r->copy->setEnabled(false);
    }
    for (QFrame *f : {m_up.fail, m_log.fail})
    {
        if (!f)
            continue;
        if (auto *t = f->findChild<QLabel *>("failReason"))
            t->setText(reason.isEmpty() ? tr("Could not decode the barcode.") : reason);
        if (auto *c = f->findChild<QLabel *>("failCode"))
            c->setText(errCode.isEmpty() ? QString("ERR_UNKNOWN") : errCode);
        f->show();
    }
}

void DecodePage::resetResult()
{
    for (const ResultUi *r : {&m_cam, &m_up})
    {
        if (!r->code)
            continue;
        r->code->setText("------");
        r->code->setStyleSheet("color:#C2CCDF;");
        if (r->badge)
            setBadge(r->badge, "\xE2\x97\x8F  Waiting", "idle");
        if (r->copy)
            r->copy->setEnabled(false);
        if (r->fail)
            r->fail->hide();
    }
    if (m_log.fail)
        m_log.fail->hide();
}

void DecodePage::setCameraActive(bool on)
{
    m_streaming = on;
    m_camera->setRunning(on);
    m_stream->setText(on ? " Stop Camera" : " Start Camera");
    setKind(m_stream, on ? "deep" : "primary");
    m_cameraBox->setEnabled(!on);
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
    m_cameraBox->addItems(names.isEmpty() ? QStringList{tr("No camera found")} : names);
}
