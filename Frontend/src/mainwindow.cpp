#include "mainwindow.h"
#include "decodepage.h"
#include "generatepage.h"
#include "icons.h"
#include "widgets.h"
#include "theme.h"
#include "barcode/BarcodeGenerator.h"
#include "scanner/CameraScanner.h"
#include "scanner/ImageScanner.h"

#include <QButtonGroup>
#include <QImageReader>
#include <QPainter>
#include <QPixmap>
#include <QScrollArea>
#include <QStackedWidget>
#include <QTemporaryDir>
#include <QTimer>
#include <QThread>

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
    sa->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    sa->setWidget(page);
    return sa;
}

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
    setWindowTitle("BarcodeHub");
    resize(1360, 900);
    setMinimumSize(960, 640);

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

    // ---- Generate tab: Backend BarcodeGenerator (EAN-8) -> preview / save / copy
    connect(m_generate, &GeneratePage::generateRequested, this, [this](const QString &code) {
        const BarcodeResult r = BarcodeGenerator().generate(code, m_generate->showDigits());
        if (r.success) m_generate->setBarcodeImage(QPixmap::fromImage(r.image));
    });

    // ---- Decode > Upload: Backend ImageScanner (+ generated samples that go through the same path)
    connect(m_decode, &DecodePage::decodeImageRequested, this, &MainWindow::decodeFile);
    connect(m_decode, &DecodePage::sampleRequested, this, &MainWindow::makeSample);

    // ---- live camera: Backend CameraScanner -> CameraView in the Decode > Camera tab
    m_scanner = new CameraScanner;
    m_camTimer = new QTimer(this);
    m_cameraThread = new QThread(this);
    m_cameraWorker = new QObject;
    m_cameraWorker->moveToThread(m_cameraThread);
    connect(m_cameraThread, &QThread::finished, m_cameraWorker, &QObject::deleteLater);
    m_cameraThread->start();
    m_camTimer->setInterval(33);
    connect(m_camTimer, &QTimer::timeout, this, &MainWindow::pumpCamera);
    connect(m_decode, &DecodePage::cameraStartRequested, this, &MainWindow::startCamera);
    connect(m_decode, &DecodePage::cameraStopRequested, this, &MainWindow::stopCamera);
    connect(m_decode, &DecodePage::cameraRefreshRequested, this, &MainWindow::refreshCameras);
    connect(m_decode, &DecodePage::autofocusRequested, this, [this] {
        const auto generation = m_cameraGeneration;
        QMetaObject::invokeMethod(m_cameraWorker, [this, generation] {
            const bool ok = m_scanner->autofocus();
            QMetaObject::invokeMethod(this, [this, generation, ok] {
                if (generation != m_cameraGeneration) return;
                m_decode->setCameraMessage(ok ? tr("Autofocus enabled. Hold the barcode steady.")
                    : tr("This camera does not support software autofocus. Adjust the distance or focus manually."));
            }, Qt::QueuedConnection);
        }, Qt::QueuedConnection);
    });
    refreshCameras();

}

MainWindow::~MainWindow()
{

    // release the webcam before the window goes away
    stopCamera();                                  
    QMetaObject::invokeMethod(m_cameraWorker, [this] { delete m_scanner; m_scanner = nullptr; }, Qt::BlockingQueuedConnection);
    m_cameraThread->quit();
    m_cameraThread->wait();
}

void MainWindow::startCamera(int index)
{
#ifndef HAVE_OPENCV
    Q_UNUSED(index);
    m_decode->setCameraActive(false);
    m_decode->setCameraMessage(tr("This build has no OpenCV, so the webcam cannot be used. "
                                  "Install OpenCV and reconfigure CMake."), true);
    return;
#else
    const auto generation = ++m_cameraGeneration;
    const QSize resolution = m_decode->cameraResolution();
    m_failedReads = 0;
    m_missFrames = 0;
    m_lastCode.clear();
    m_readPending = false;
    QMetaObject::invokeMethod(m_cameraWorker, [this, index, resolution, generation] {
        QString error;
        const bool opened = m_scanner->openCamera(index, &error, resolution);
        QMetaObject::invokeMethod(this, [this, generation, opened, error] {
            if (generation != m_cameraGeneration) return;
            m_decode->setCameraActive(opened);
            if (opened) m_camTimer->start();
            else m_decode->setCameraMessage(error, true);
        }, Qt::QueuedConnection);
    }, Qt::QueuedConnection);
#endif
}

void MainWindow::stopCamera()
{
    if (m_camTimer) m_camTimer->stop();
    ++m_cameraGeneration;
    m_readPending = false;
    m_decode->setCameraActive(false);
    if (m_cameraWorker) QMetaObject::invokeMethod(m_cameraWorker, [this] { m_scanner->closeCamera(); }, Qt::QueuedConnection);
}

void MainWindow::pumpCamera()
{
    if (m_readPending) return;
    m_readPending = true;
    const auto generation = m_cameraGeneration;
    const bool decode = !m_decode->cameraView()->frozen();
    QMetaObject::invokeMethod(m_cameraWorker, [this, generation, decode] {
        const CameraFrameResult result = m_scanner->readFrame(decode);
        QMetaObject::invokeMethod(this, [this, generation, result] {
            if (generation != m_cameraGeneration) return;
            m_readPending = false;
            acceptCameraFrame(result);
        }, Qt::QueuedConnection);
    }, Qt::QueuedConnection);
}

void MainWindow::refreshCameras()
{
    if (m_decode->cameraActive()) return;
    QMetaObject::invokeMethod(m_cameraWorker, [this] {
        const QStringList names = CameraScanner::availableCameras();
        QMetaObject::invokeMethod(this, [this, names] {
            if (!m_decode->cameraActive()) m_decode->setCameraList(names);
        }, Qt::QueuedConnection);
    }, Qt::QueuedConnection);
}

void MainWindow::acceptCameraFrame(const CameraFrameResult &r)
{
    if (r.success && !r.frame.isNull()) {
        m_failedReads = 0;
        if (m_decode->cameraView()->frozen()) return;
        // mirrored for display only (like a mirror / selfie view); the original frame stays unflipped for decoding
        CameraView *view = m_decode->cameraView();
        const bool mirror = m_decode->mirrorCamera();
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        view->setFrame(QPixmap::fromImage(mirror ? r.frame.flipped(Qt::Horizontal) : r.frame));
#else
        view->setFrame(QPixmap::fromImage(mirror ? r.frame.mirrored(true, false) : r.frame));
#endif

        if (!r.decodedText.isEmpty()) {              // Backend/Scanner found and decoded an EAN-8 barcode in this frame
            m_missFrames = 0;
            const QRect b = r.box;
            view->setDetection(mirror ? QRect(r.frame.width() - (b.x() + b.width()), b.y(), b.width(), b.height()) : b, r.decodedText);
            if (r.decodedText != m_lastCode) {
                m_lastCode = r.decodedText;
                DecodePage::ScanInfo info;
                info.latencyMs = r.decodeMs;
                info.box = b.size();
                info.payload = BarcodeGenerator::ean8Payload(r.decodedText);
                m_decode->showResult(r.decodedText, info);
                m_decode->setCameraMessage(tr("Barcode detected: %1").arg(r.decodedText));
            }
        } else if (++m_missFrames > 8) {
            view->clearDetection();                  // barcode left the picture
            m_lastCode.clear();
        }
        return;
    }
    if (++m_failedReads >= 30) {                     // ~1 s without a frame: camera unplugged / busy
        stopCamera();
        m_decode->setCameraActive(false);
        m_decode->setCameraMessage(r.error.isEmpty() ? tr("Camera stopped delivering frames.") : r.error, true);
    }
}

void MainWindow::decodeFile(const QString &path)
{
    QImageReader reader(path);
    reader.setAutoTransform(true);                   // respect EXIF rotation of phone photos
    const QImage img = reader.read();
    if (img.isNull()) {
        m_decode->showFailure(tr("Could not read the image file: %1").arg(reader.errorString()), "ERR_BAD_IMAGE");
        return;
    }
    const ImageScanResult r = ImageScanner().scan(img);
    if (!r.success) {
        const QString code = r.error.contains("OpenCV") ? "ERR_NO_OPENCV"
                           : r.error.contains("Check digit") ? "ERR_CHECK_DIGIT" : "ERR_NO_BARCODE";
        m_decode->showFailure(r.error, code);
        return;
    }
    DecodePage::ScanInfo info;
    info.latencyMs = r.latencyMs;
    info.box = r.box.size();
    info.payload = BarcodeGenerator::ean8Payload(r.text);
    m_decode->showResult(r.text, info);
}

void MainWindow::makeSample(const QString &code, bool damaged)
{
    BarcodeResult r = BarcodeGenerator().generate(code, true);
    if (!r.success) {
        m_decode->showFailure(r.error, "ERR_SAMPLE");
        return;
    }
    if (damaged) {                                   // scratch the bars so the check/structure no longer matches
        QPainter p(&r.image);
        p.fillRect(250, 20, 30, 170, Qt::white);
        p.fillRect(380, 20, 12, 170, Qt::black);
    }
    if (!m_tmp) m_tmp = std::make_unique<QTemporaryDir>();
    const QString path = m_tmp->filePath(QString("sample_%1%2.png").arg(code, damaged ? "_damaged" : ""));
    if (!m_tmp->isValid() || !r.image.save(path, "PNG")) {
        m_decode->showFailure(tr("Could not write the sample image."), "ERR_SAMPLE");
        return;
    }
    m_decode->loadImage(path);                       // shows file name / size, like a real upload
    decodeFile(path);
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
    connect(m_nav, &QButtonGroup::idClicked, this, [this](int id) {
        if (id != 1) m_decode->stopCamera();         // leaving Decode: switch the webcam off
        m_pages->setCurrentIndex(id);
    });
    il->addStretch();
    l->addWidget(inner, 1);

    auto *foot = new QWidget;
    auto *fl = vbox(foot, 12, 0, 12, 14, 0);
    auto *chipRow = panel();
    auto *cr = hbox(chipRow, 12, 9, 12, 9, 8);
    cr->addWidget(T("\xE2\x97\x8F", 9, 400, "#0B4BCB"));
    cr->addWidget(T("EAN-8 reader", 11, 500, "#434655", true));
    cr->addStretch();
    cr->addWidget(T("OpenCV", 11, 400, "#737686", true));
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
    auto *themeBtn = btn("", "square", "moon");
    themeBtn->setObjectName("themeToggle");
    themeBtn->setToolTip(tr("Toggle Night Mode"));
    connect(themeBtn, &QPushButton::clicked, this, [this, themeBtn] {
        Theme::toggleNightMode(this);
        themeBtn->setIcon(makeIcon(Theme::isNightMode() ? "sun" : "moon",
                                   Theme::isNightMode() ? QColor("#FBBF24") : QColor("#334155"), 18));
    });
    l->addWidget(themeBtn, 0, Qt::AlignVCenter);
    l->addSpacing(8);
    l->addWidget(iconChip("server", "OpenCV Ready", "#EFF4FF", "#434655", "#E1E8F7", true, 12, 8), 0, Qt::AlignVCenter);
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
