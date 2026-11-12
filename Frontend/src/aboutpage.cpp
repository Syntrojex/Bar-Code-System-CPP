#include "aboutpage.h"
#include "icons.h"
#include "widgets.h"

#include <QDesktopServices>
#include <QUrl>

namespace {

QLabel *body(const QString &text)
{
    auto *l = T(text, 14, 400, "#434655");
    l->setWordWrap(true);
    return l;
}

// card with a title row (icon tile + heading); returns the inner layout for content
QVBoxLayout *section(QVBoxLayout *parent, const QString &icon, const QString &title, QFrame **out = nullptr)
{
    auto *c = card();
    auto *l = vbox(c, 28, 24, 28, 24, 12);
    auto *head = hrow(12);
    head->addWidget(iconTile(icon, QColor("#0B4BCB"), 36, 18));
    head->addWidget(T(title, 18, 600, "#0B1C30"));
    head->addStretch();
    l->addLayout(head);
    parent->addWidget(c);
    if (out) *out = c;
    return l;
}

QFrame *feature(const QString &icon, const QString &title, const QString &text)
{
    auto *f = panel("panelB");
    auto *l = hbox(f, 14, 14, 14, 14, 12);
    l->addWidget(iconTile(icon, QColor("#0B4BCB"), 36, 18), 0, Qt::AlignTop);
    auto *col = vcol(3);
    col->addWidget(T(title, 14, 600, "#0B1C30"));
    col->addWidget(body(text));
    l->addLayout(col, 1);
    return f;
}

QFrame *member(const QString &name, const QString &roll, const QString &role,
               const QString &work, const QString &github)
{
    auto *f = panel("panelB");
    auto *l = vbox(f, 16, 16, 16, 16, 6);
    auto *top = hrow(10);
    top->addWidget(iconTile("user", QColor("#0B4BCB"), 36, 18));
    auto *names = vcol(0);
    names->addWidget(T(name, 15, 600, "#0B1C30"));
    names->addWidget(T(roll, 11, 500, "#737686", true));
    top->addLayout(names, 1);
    l->addLayout(top);
    l->addWidget(pill(role, "tint"), 0, Qt::AlignLeft);
    l->addWidget(body(work));
    auto *gh = new QPushButton("  @" + github);
    gh->setCursor(Qt::PointingHandCursor);
    gh->setFlat(true);
    gh->setStyleSheet("QPushButton { text-align:left; border:none; background:transparent; color:#0B4BCB; font-size:12px; padding:2px 0; }"
                      "QPushButton:hover { text-decoration:underline; }");
    QObject::connect(gh, &QPushButton::clicked, gh, [github] {
        QDesktopServices::openUrl(QUrl("https://github.com/" + github));
    });
    l->addWidget(gh, 0, Qt::AlignLeft);
    return f;
}

QFrame *infoRow(const QString &k, const QString &v)
{
    auto *f = new QFrame;
    auto *l = hbox(f, 0, 4, 0, 4, 12);
    auto *key = T(k, 13, 500, "#737686");
    key->setFixedWidth(130);
    l->addWidget(key);
    auto *val = T(v, 13, 500, "#0B1C30");
    val->setWordWrap(true);
    l->addWidget(val, 1);
    return f;
}

} // namespace

AboutPage::AboutPage(QWidget *parent) : QWidget(parent)
{
    setObjectName("page");
    auto *root = vbox(this, 24, 22, 24, 24, 20);

    // ---------------------------------------------------------------- header
    auto *head = hrow();
    auto *titles = vcol(2);
    auto *crumb = hrow(8);
    crumb->addWidget(T("PROJECT INFORMATION", 11, 500, "#737686", true));
    crumb->addWidget(T("\xE2\x80\xA2", 11, 500, "#C3C6D7"));
    crumb->addWidget(T("ABOUT-05", 11, 500, "#0B4BCB", true));
    crumb->addStretch();
    titles->addLayout(crumb);
    titles->addWidget(T(tr("About Project"), 26, 600, "#0B1C30"));
    head->addLayout(titles);
    head->addStretch();
    head->addWidget(pill("<span style='color:#0B4BCB'>\xE2\x97\x8F</span>&nbsp;&nbsp;v1.0.0 \xC2\xB7 Increment 1", "white"), 0, Qt::AlignBottom);
    root->addLayout(head);

    // ---------------------------------------------------------------- overview
    {
        auto *l = section(root, "info", tr("Overview"));
        l->addWidget(body(tr(
            "BarcodeHub is a C++ desktop application for barcode generation, image decoding and real-time "
            "camera scanning. It is a collaborative academic project for the Introduction to Software "
            "Engineering (ISE) course at FAST-NUCES, developed incrementally with a focus on reliability, "
            "usability and maintainable architecture. The goal is to simplify product identification and "
            "barcode-related operations in retail environments through a simple desktop interface.")));
        auto *chips = hrow(8);
        chips->addWidget(chip("C++17", "blue"));
        chips->addWidget(chip("Qt 6 Widgets", "blue"));
        chips->addWidget(chip("OpenCV", "blue"));
        chips->addWidget(chip("CMake", "neutral"));
        chips->addWidget(chip("Windows x64", "neutral"));
        chips->addStretch();
        l->addLayout(chips);
    }

    // ---------------------------------------------------------------- features
    {
        auto *l = section(root, "bolt", tr("What it does"));
        auto *g1 = hrow(12), *g2 = hrow(12);
        g1->addWidget(feature("barcode", tr("Barcode Generation"),
                              tr("Turn a six-digit product code into a barcode, preview it, then save or copy the image.")), 1);
        g1->addWidget(feature("camera", tr("Camera Scanning"),
                              tr("Pick a camera and resolution, scan live, and freeze, mirror or capture frames.")), 1);
        g2->addWidget(feature("image", tr("Image Decoding"),
                              tr("Upload a PNG, JPEG or BMP and decode the code, with format and checksum validation.")), 1);
        g2->addWidget(feature("history", tr("Scan Logs & CSV Export"),
                              tr("Successful scans are kept in a session log that can be exported as CSV.")), 1);
        l->addLayout(g1);
        l->addLayout(g2);
    }

    // ---------------------------------------------------------------- format
    {
        auto *l = section(root, "hash", tr("Supported barcode format"));
        l->addWidget(body(tr(
            "Increment 1 uses a project-specific, EAN-8-sized format. The input is exactly six digits "
            "and the first digit must not be zero. The encoded payload is 0 + code + a Mod-10 check digit, "
            "and decoding validates both the format and the checksum.")));
        auto *eg = panel("panelB");
        auto *el = hbox(eg, 16, 12, 16, 12, 12);
        el->addWidget(caption("EXAMPLE", true));
        el->addWidget(T("123456  \xE2\x86\x92  0 123456 5  =  01234565", 14, 500, "#0B1C30", true));
        el->addStretch();
        l->addWidget(eg);
        auto *note = T(tr("It does not claim support for arbitrary EAN-13, QR or Code 128 barcodes. "
                          "SVG export and thermal printing are currently disabled."),
                       12, 400, "#737686");
        note->setWordWrap(true);
        l->addWidget(note);
    }

    // ---------------------------------------------------------------- architecture + stack
    {
        auto *row = hrow(20);
        auto *left = vcol(0), *right = vcol(0);
        QFrame *c1 = nullptr, *c2 = nullptr;
        auto *l1 = section(left, "braces", tr("Architecture"), &c1);
        l1->addWidget(body(tr(
            "The code is split into a Backend (barcode generation, decoding, image and camera scanners) and a "
            "Frontend (the Qt interface). They are built as two static libraries, barcode_core and barcode_ui, "
            "plus the BarcodeHub executable and a regression test target.")));
        l1->addStretch();
        auto *l2 = section(right, "server", tr("Technology stack"), &c2);
        l2->addWidget(infoRow("Language", "C++17"));
        l2->addWidget(infoRow("Interface", "Qt 6 Widgets"));
        l2->addWidget(infoRow("Imaging", "OpenCV 4 (camera + decode)"));
        l2->addWidget(infoRow("Build", "CMake, MinGW-w64 / MSVC"));
        l2->addWidget(infoRow("Tooling", "PowerShell, Git & GitHub Actions"));
        row->addLayout(left, 1);
        row->addLayout(right, 1);
        root->addLayout(row);
    }

    // ---------------------------------------------------------------- academic
    {
        auto *l = section(root, "folder", tr("Academic information"));
        l->addWidget(infoRow("Course", "Introduction to Software Engineering (ISE)"));
        l->addWidget(infoRow("Institution", "FAST-NUCES"));
        l->addWidget(infoRow("Instructor", "Professor Rana Zeeshan"));
        l->addWidget(infoRow("Methodology", "Incremental development"));
        l->addWidget(infoRow("Repository", "Syntrojex/Bar-Code-System-CPP"));
        l->addWidget(infoRow("Status", "Increment 1 completed"));
    }

    // ---------------------------------------------------------------- team
    {
        auto *l = section(root, "user", tr("Project team"));
        auto *g1 = hrow(12), *g2 = hrow(12);
        g1->addWidget(member("Muhammad Mustafa Amir", "25L-3107", "Project Lead",
                             tr("Management, integration, error handling, CMake build and the live scanner."),
                             "Syntrojex"), 1);
        g1->addWidget(member("Ali Abdullah", "25L-3022", "Developer",
                             tr("UI development, barcode generation, image processing and input handling."),
                             "aliabdullah-se"), 1);
        g2->addWidget(member("Farzam Zeeshan", "25L-3049", "Developer",
                             tr("UI development, barcode reading and decoding, invalid input and file handling."),
                             "farzamzeeshan1"), 1);
        g2->addWidget(member("Shauzab Mukhtar", "25L-3084", "Developer",
                             tr("Camera initialisation and frame capture, live scanner and barcode reading."),
                             "shauzab-72"), 1);
        l->addLayout(g1);
        l->addLayout(g2);
    }

    root->addStretch();
}
