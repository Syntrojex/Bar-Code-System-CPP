#include "generatepage.h"
#include "icons.h"
#include "widgets.h"

#include <QAction>
#include <QApplication>
#include <QClipboard>
#include <QFileDialog>
#include <QLineEdit>
#include <QMessageBox>
#include <QRegularExpression>

#include <QRegularExpressionValidator>
#include <QSlider>
#include <QTime>
#include <QTimer>

// --------------------------------------------------- local helpers
// Project rule: exactly six digits, first digit must not be zero.
static bool isValidCode(const QString &s)
{
    static const QRegularExpression re("^[1-9][0-9]{5}$");
    return re.match(s).hasMatch();
}



static void showInputError(QLineEdit *input, QLabel *hint, QLabel *icon, const QString &msg)
{
    setKind(input, "error");
    hint->setText(msg);
    hint->setStyleSheet("font-size:13px; color:#BA1A1A;");
    icon->setPixmap(iconPixmap("warn", QColor("#BA1A1A"), 16));
}

GeneratePage::GeneratePage(QWidget *parent) : QWidget(parent)
{
    setObjectName("page");
    auto *root = vbox(this, 24, 22, 24, 24, 20);

    // ---------------------------------------------------------------- header
    auto *head = hrow();
    auto *titles = vcol(2);
    auto *crumb = hrow(8);
    crumb->addWidget(T("OPERATION STAGE", 11, 500, "#737686", true));
    crumb->addWidget(T("\xE2\x80\xA2", 11, 500, "#C3C6D7"));
    crumb->addWidget(T("SYMB-GEN-01", 11, 500, "#0B4BCB", true));
    crumb->addStretch();
    titles->addLayout(crumb);
    titles->addWidget(T(tr("Generate Barcode"), 26, 600, "#0B1C30"));
    head->addLayout(titles);
    head->addStretch();
    auto *p1 = pill("<span style='color:#0B4BCB'>\xE2\x97\x8F</span>&nbsp;&nbsp;Generator Ready", "white");
    head->addWidget(p1, 0, Qt::AlignBottom);
    root->addLayout(head);

    auto *body = hrow(20);

    // ----------- left: configuration
    auto *cfg = card();
    cfg->setFixedWidth(394);
    auto *cl = vbox(cfg, 28, 28, 28, 28, 12);

    auto *r1 = hrow();
    r1->addWidget(T(tr("Barcode Configuration"), 18, 600, "#0B1C30"));
    r1->addStretch();
    r1->addWidget(chip("1D Linear", "neutral"));
    cl->addLayout(r1);
    auto *desc = T(tr("Enter numeric values to generate standard industrial barcode symbology."), 14, 400, "#434655");
    desc->setWordWrap(true);
    cl->addWidget(desc);
    cl->addSpacing(8);

    auto *r2 = hrow();
    auto *lbl = new QLabel(QString("<span style='font-size:14px;font-weight:600;color:#0B1C30'>Enter 6-digit code</span>"
                                   " <span style='color:#BA1A1A;font-weight:700'>*</span>"));
    r2->addWidget(lbl);
    r2->addStretch();
    m_counter = chip("0 / 6", "blue");
    r2->addWidget(m_counter);
    cl->addLayout(r2);

    m_input = new QLineEdit;
    m_input->setObjectName("codeInput");
    m_input->setMaxLength(6);
    m_input->setPlaceholderText("123456");
    // digits only; the "first digit must not be zero" rule is checked in onTextChanged() so the user gets feedback
    m_input->setValidator(new QRegularExpressionValidator(QRegularExpression("[0-9]{0,6}"), m_input));
    QFont mf = monoFont(20);
    mf.setLetterSpacing(QFont::AbsoluteSpacing, 3);
    m_input->setFont(mf);
    m_input->addAction(makeIcon("hash", QColor("#737686"), 18), QLineEdit::LeadingPosition);
    m_checkAction = m_input->addAction(makeIcon("check", QColor("#0B4BCB"), 20), QLineEdit::TrailingPosition);
    m_checkAction->setVisible(false);
    cl->addWidget(m_input);

    auto *hr = hrow(8);
    m_hintIcon = iconLabel("info", QColor("#0B4BCB"), 16);
    m_hint = T(tr("Must be exactly 6 digits, first digit not zero"), 13, 400, "#434655");
    hr->addWidget(m_hintIcon);
    hr->addWidget(m_hint);
    hr->addStretch();
    m_validChip = chip("Format OK", "blue");
    m_validChip->hide();
    hr->addWidget(m_validChip);
    cl->addLayout(hr);
    cl->addSpacing(4);

    auto *br = hrow(10);
    m_generate = btn(tr("Generate Barcode"), "primary", "barcode");
    m_generate->setMinimumHeight(44);
    m_generate->setEnabled(false);
    m_clear = btn(tr("Clear"), "outline", "backspace");
    m_clear->setMinimumHeight(44);
    br->addWidget(m_generate, 1);
    br->addWidget(m_clear);
    cl->addLayout(br);
    cl->addSpacing(4);

    // ---- symbology parameters panel
    auto *sym = panel();
    auto *sl = vbox(sym, 16, 16, 16, 16, 10);
    auto *sh = hrow();
    sh->addWidget(T(tr("Symbology Parameters"), 14, 500, "#0B1C30"));
    sh->addStretch();
    sh->addWidget(T("SPEC ISO/IEC 15420", 11, 400, "#737686", true));
    sl->addLayout(sh);
    sl->addWidget(T("LINEAR STANDARD", 11, 600, "#737686"));

    // EAN-8 is the only symbology: 0 + 6-digit code + check digit
    m_ean8 = new RadioCard("EAN-8", "0 + code + check digit");
    m_ean8->setSelected(true);
    sl->addWidget(m_ean8);

    auto *sr = hrow(16);
    auto addSlider = [&](const QString &name, int mn, int mx, int val, QLabel *&valLabel, QSlider *&slider) {
        auto *col = vcol(6);
        auto *top = hrow();
        top->addWidget(T(name, 12, 500, "#434655", true));
        top->addStretch();
        valLabel = T("", 12, 500, "#0B1C30", true);
        top->addWidget(valLabel);
        col->addLayout(top);
        slider = new QSlider(Qt::Horizontal);
        slider->setRange(mn, mx);
        slider->setValue(val);
        slider->setCursor(Qt::PointingHandCursor);
        col->addWidget(slider);
        sr->addLayout(col, 1);
    };
    addSlider("Bar Width", 10, 40, 22, m_barVal, m_barSlider);
    addSlider("Quiet Zone", 4, 20, 10, m_quietVal, m_quietSlider);
    // not connected to the renderer yet -> disabled so they don't look functional
    m_barSlider->setEnabled(false);
    m_quietSlider->setEnabled(false);
    m_barSlider->setToolTip(tr("Bar width is fixed for this increment"));
    m_quietSlider->setToolTip(tr("Quiet zone is fixed for this increment"));
    // initialise display labels to match the slider default values
    m_barVal->setText(QString("%1 px").arg(22 / 10.0, 0, 'f', 1));
    m_quietVal->setText(QString("%1x").arg(10));
    sl->addLayout(sr);

    auto *tr3 = hrow(12);
    auto *tc = vcol(2);
    tc->addWidget(T(tr("Include Human-Readable OCR"), 14, 500, "#0B1C30"));
    auto *ocrSub = T(tr("Render the full EAN-8 digits under the bars"), 12, 400, "#434655");
    tc->addWidget(ocrSub);
    tr3->addLayout(tc, 1);
    m_ocr = new ToggleSwitch(true);
    tr3->addWidget(m_ocr);
    sl->addLayout(tr3);
    cl->addWidget(sym);

    auto *note = panel("panelB");
    auto *nl = hbox(note, 14, 12, 14, 12, 10);
    nl->addWidget(iconLabel("gauge", QColor("#0B4BCB"), 18), 0, Qt::AlignTop);
    auto *nt = new QLabel("Linear rasterizer ready.");
    nt->setWordWrap(true);
    nt->setStyleSheet("font-size:12px; color:#434655; font-family:" + monoFamily() + ";");
    nl->addWidget(nt, 1);
    cl->addWidget(note);
    cl->addStretch();

    body->addWidget(cfg, 0, Qt::AlignTop);

    // ================================================================ right column
    auto *right = vcol(20);

    auto *pv = card();
    auto *pl = vbox(pv, 24, 24, 24, 24, 14);
    auto *ph = hrow(12);
    ph->addWidget(T(tr("Barcode Preview"), 18, 600, "#0B1C30"));
    ph->addWidget(iconChip("print", "Lossless PNG", "#E1EAFF", "#1D4ED8", QString(), true, 11, 6));
    ph->addStretch();
    m_stageSeg = new SegBar({"Render Stage", "Awaiting State"}, {}, 0, true);
    ph->addWidget(m_stageSeg);
    pl->addLayout(ph);

    m_view = new BarcodeView;
    pl->addWidget(m_view, 1);

    auto *ab = hrow(10);
    m_save = btn(tr("Save as PNG"), "primary", "download");
    m_copy = btn(tr("Copy Image"), "outline", "copy");
    auto *svg = btn(tr("Export SVG"), "tint", "braces");
    for (auto *b : {m_save, m_copy, svg}) b->setMinimumHeight(40);
    ab->addWidget(m_save);
    ab->addWidget(m_copy);
    ab->addWidget(svg);
    ab->addStretch();
    pl->addLayout(ab);
    auto *ab2 = hrow();
    ab2->addStretch();
    auto *thermal = btn(tr("Thermal Print"), "ghost", "print");
    thermal->setMinimumHeight(40);
    ab2->addWidget(thermal);
    pl->addLayout(ab2);

    // no backend behind these two yet
    svg->setEnabled(false);
    svg->setToolTip(tr("Not implemented yet"));
    thermal->setEnabled(false);
    thermal->setToolTip(tr("Not implemented yet"));

    auto *info = panel();
    auto *il = hbox(info, 16, 12, 16, 12, 12);
    auto *infoL = new QLabel("<span style='color:#434655'>Dimensions: 600\xC3\x97" "240px &nbsp;\xE2\x80\xA2&nbsp; Format: PNG"
                             " &nbsp;\xE2\x80\xA2&nbsp; Color: Black (#000000)</span>");
    infoL->setWordWrap(true);
    infoL->setStyleSheet("font-size:12px; font-family:" + monoFamily() + ";");
    il->addWidget(iconLabel("image", QColor("#0B4BCB"), 16), 0, Qt::AlignTop);
    il->addWidget(infoL, 1);
    il->addWidget(T("Buffer: Lossless", 12, 400, "#737686", true), 0, Qt::AlignTop);
    pl->addWidget(info);
    right->addWidget(pv);

    // ---- session card
    auto *sc = card();
    auto *scl = vbox(sc, 24, 20, 24, 22, 14);
    auto *sh2 = hrow();
    sh2->addWidget(T(tr("Active Session Sequence"), 16, 600, "#0B1C30"));
    sh2->addStretch();
    sh2->addWidget(T("Last 3 Generated", 12, 400, "#737686", true));
    scl->addLayout(sh2);
    m_sessionRow = hrow(12);
    scl->addLayout(m_sessionRow);
    right->addWidget(sc);
    right->addStretch();

    body->addLayout(right, 1);
    root->addLayout(body, 1);

    // ---------------------------------------------------------------- initial state (empty, no fake data)
    m_sessions.clear();
    rebuildSessions();
    m_view->clear();
    m_save->setEnabled(false);
    m_copy->setEnabled(false);

    // ---------------------------------------------------------------- wiring
    connect(m_input, &QLineEdit::textChanged, this, &GeneratePage::onTextChanged);
    connect(m_input, &QLineEdit::returnPressed, this, [this] { if (m_generate->isEnabled()) onGenerate(); });
    connect(m_input, &QLineEdit::editingFinished, this, [this] {
        const QString t = m_input->text();
        // leading-zero error is already shown by onTextChanged(), don't overwrite it
        if (!t.isEmpty() && t.size() < 6 && t.at(0) != QLatin1Char('0'))
            showInputError(m_input, m_hint, m_hintIcon, tr("Code must be exactly 6 digits"));
    });
    connect(m_generate, &QPushButton::clicked, this, &GeneratePage::onGenerate);
    connect(m_clear, &QPushButton::clicked, this, &GeneratePage::onClear);
    connect(m_save, &QPushButton::clicked, this, &GeneratePage::onSave);
    connect(m_copy, &QPushButton::clicked, this, &GeneratePage::onCopy);
    connect(m_ocr, &ToggleSwitch::toggled, m_view, &BarcodeView::setShowDigits);
    connect(m_ocr, &ToggleSwitch::toggled, this, [this] {
        const QString code = currentCode();
        if (!code.isEmpty()) emit generateRequested(code);          // re-render the backend image with / without digits
    });
    connect(m_stageSeg, &SegBar::currentChanged, this, [this](int i) { m_view->setAwaiting(i == 1); });
    connect(m_ean8, &RadioCard::clicked, this, [this] { m_ean8->setSelected(true); });
    connect(m_barSlider, &QSlider::valueChanged, this, [this](int v) { m_barVal->setText(QString("%1 px").arg(v / 10.0, 0, 'f', 1)); });
    connect(m_quietSlider, &QSlider::valueChanged, this, [this](int v) { m_quietVal->setText(QString("%1x").arg(v)); });
}

void GeneratePage::setBarcodeImage(const QPixmap &pm) { m_view->setImage(pm); }
bool GeneratePage::showDigits() const { return m_ocr->isChecked(); }

void GeneratePage::setCode(const QString &code, bool generate)
{
    if (code.isEmpty()) { m_input->clear(); return; }
    if (!generate) { m_input->setText(code); return; }   // prefill only; onTextChanged() shows any problem

    if (!isValidCode(code)) {
        m_input->clear();                                  // resets hint state first ...
        showInputError(m_input, m_hint, m_hintIcon,        // ... then shows the error
                       tr("Invalid code: must be exactly 6 digits, first digit not zero"));
        return;
    }
    m_input->setText(code);
    onGenerate();                                          // same path as pressing the button
}

void GeneratePage::onTextChanged(const QString &t)
{
    m_counter->setText(QString("%1 / 6").arg(t.size()));
    const bool valid = isValidCode(t);
    m_generate->setEnabled(valid);
    m_validChip->setVisible(valid);
    m_checkAction->setVisible(valid);

    setKind(m_input, "");
    m_hint->setText(tr("Must be exactly 6 digits, first digit not zero"));
    m_hint->setStyleSheet("font-size:13px; color:#434655;");
    m_hintIcon->setPixmap(iconPixmap("info", QColor("#0B4BCB"), 16));

    if (!t.isEmpty() && t.at(0) == QLatin1Char('0'))
        showInputError(m_input, m_hint, m_hintIcon, tr("First digit must not be zero"));
}

void GeneratePage::onGenerate()
{
    const QString code = m_input->text();
    if (!isValidCode(code)) {                              // defensive: button/Enter should already be blocked
        m_generate->setEnabled(false);
        showInputError(m_input, m_hint, m_hintIcon,
                       tr("Invalid code: must be exactly 6 digits, first digit not zero"));
        return;
    }

    m_view->setCode(code);
    m_stageSeg->setCurrent(0);
    m_view->setAwaiting(false);
    setProperty("genCode", code);
    m_save->setEnabled(true);
    m_copy->setEnabled(true);

    for (int i = m_sessions.size() - 1; i >= 0; --i)
        if (m_sessions[i].code == code) m_sessions.removeAt(i);
    m_sessions.prepend({code, "EAN-8", QTime::currentTime().toString("HH:mm:ss")});
    while (m_sessions.size() > 3) m_sessions.removeLast();
    rebuildSessions();
    emit generateRequested(code);
}

void GeneratePage::onClear()
{
    m_input->clear();
    m_view->clear();
    setProperty("genCode", QString());
    m_save->setEnabled(false);
    m_copy->setEnabled(false);
    m_input->setFocus();
}

void GeneratePage::onSave()
{
    const QString code = property("genCode").toString();
    if (code.isEmpty()) {
        QMessageBox::warning(this, tr("Save barcode"), tr("Generate a barcode first."));
        return;
    }
    const QPixmap pm = m_view->renderBarcode();
    if (pm.isNull()) {
        QMessageBox::warning(this, tr("Save barcode"), tr("Could not render the barcode image."));
        return;
    }
    QString path = QFileDialog::getSaveFileName(this, tr("Save barcode"), code + ".png", tr("PNG image (*.png)"));
    if (path.isEmpty()) return;                            // user cancelled
    if (!path.endsWith(".png", Qt::CaseInsensitive)) path += ".png";
    if (!pm.save(path, "PNG"))
        QMessageBox::warning(this, tr("Save barcode"),
                             tr("Could not save the file:\n%1\n\nCheck the folder permissions and free disk space.").arg(path));
}

void GeneratePage::onCopy()
{
    if (m_copy->property("copied").toBool()) return;       // double-click guard
    const QPixmap pm = m_view->renderBarcode();
    if (pm.isNull()) {
        QMessageBox::warning(this, tr("Copy image"), tr("Could not render the barcode image."));
        return;
    }
    QApplication::clipboard()->setPixmap(pm);
    const QString old = m_copy->text();
    m_copy->setProperty("copied", true);
    m_copy->setText(tr(" Copied!"));
    QTimer::singleShot(1500, this, [this, old] {
        m_copy->setText(old);
        m_copy->setProperty("copied", false);
    });
}

void GeneratePage::rebuildSessions()
{
    while (m_sessionRow->count()) {
        QLayoutItem *it = m_sessionRow->takeAt(0);
        if (it->widget()) it->widget()->deleteLater();
        delete it;
    }
    if (m_sessions.isEmpty()) {
        m_sessionRow->addWidget(T(tr("No barcodes generated yet"), 12, 400, "#737686", true), 1);
        return;
    }
    for (int i = 0; i < m_sessions.size(); ++i) {
        const Session &s = m_sessions[i];
        auto *b = new QPushButton;
        b->setObjectName("tile");
        b->setProperty("active", i == 0);
        b->setCursor(Qt::PointingHandCursor);
        b->setMinimumHeight(70);
        auto *l = vbox(b, 14, 10, 14, 10, 4);
        auto *top = hrow();
        auto *code = T(s.code, 16, 600, i == 0 ? "#0B4BCB" : "#0B1C30", true);
        top->addWidget(code);
        top->addStretch();
        top->addWidget(chip(i == 0 ? "Active" : "Logged", i == 0 ? "blue" : "neutral"));
        l->addLayout(top);
        l->addWidget(T(s.symbology + " \xE2\x80\xA2 " + s.time, 11, 400, "#737686", true));
        for (auto *w : b->findChildren<QWidget *>()) w->setAttribute(Qt::WA_TransparentForMouseEvents);
        connect(b, &QPushButton::clicked, this, [this, c = s.code] { setCode(c, true); });
        m_sessionRow->addWidget(b, 1);
    }
}
