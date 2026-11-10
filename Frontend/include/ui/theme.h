#pragma once
#include <QApplication>
#include <QLabel>
#include <QString>
#include <QWidget>

// Design tokens + global stylesheet, taken from the Stitch "Precision Utility" export.
// NOTE: font family/size are NOT set here (QSS would override setFont) – see main.cpp.
namespace Theme {

inline bool &isNightMode()
{
    static bool s_night = false;
    return s_night;
}

inline QString styleSheet()
{
    return QStringLiteral(R"(
* { color: #0B1C30; }
QWidget#root, QWidget#page, QStackedWidget { background: #F8F9FF; }
QToolTip { background: #213145; color: #EAF1FF; border: none; padding: 5px 8px; border-radius: 6px; }

/* ---------- chrome ---------- */
QFrame#sidebar { background: #FFFFFF; border: none; border-right: 1px solid #E4E8F1; }
QFrame#topbar  { background: #FFFFFF; border: none; border-bottom: 1px solid #E4E8F1; }
QFrame#brandRow{ background: transparent; border: none; border-bottom: 1px solid #E4E8F1; }

QPushButton#nav { text-align: left; padding: 0 12px; min-height: 40px; max-height: 40px; border: 1px solid transparent;
                  border-radius: 8px; color: #434655; font-size: 14px; font-weight: 500; background: transparent; }
QPushButton#nav:hover   { background: #EFF4FF; color: #0B1C30; }
QPushButton#nav:checked { background: #EFF4FF; border: 1px solid #D3DEF5; color: #004AC6; font-weight: 600; }

/* ---------- surfaces ---------- */
QFrame#card   { background: #FFFFFF; border: 1px solid #E4E8F1; border-radius: 12px; }
QFrame#panel  { background: #EFF4FF; border: none; border-radius: 10px; }
QFrame#panelB { background: #EFF4FF; border: 1px solid #E1E9FA; border-radius: 10px; }
QFrame#white  { background: #FFFFFF; border: 1px solid #E4E8F1; border-radius: 10px; }
QFrame#tileSel{ background: #E5EEFF; border: 1px solid #C9D9F8; border-radius: 10px; }
QFrame#fail   { background: #FFEDEB; border: 1px solid #F8D0CC; border-radius: 12px; }
QFrame#hline  { background: #EDF0F6; border: none; min-height: 1px; max-height: 1px; }
QFrame#iconTile { background: #E5EEFF; border: none; border-radius: 10px; }
QFrame#avatar { background: #0B4BCB; border: none; border-radius: 20px; }

/* ---------- chips & pills ---------- */
QLabel#chip { border: 1px solid transparent; border-radius: 6px; padding: 2px 8px; font-family: "JetBrains Mono","Consolas","DejaVu Sans Mono",monospace;
              font-size: 11px; font-weight: 600; }
QLabel#chip[kind="blue"]    { background: #E1EAFF; color: #1F4FCF; border-color: #E1EAFF;  border-radius: 6px; }
QLabel#chip[kind="neutral"] { background: #EEF2FA; color: #434655; border-color: #E2E7F1;  border-radius: 6px; }
QLabel#chip[kind="white"]   { background: #FFFFFF; color: #434655; border-color: #FFFFFF;  border-radius: 6px; }
QLabel#chip[kind="green"]   { background: #E3F6EC; color: #0B7A4B; border-color: #E3F6EC;  border-radius: 6px; }

QLabel#pill { border: 1px solid transparent; border-radius: 12px; padding: 3px 11px; font-size: 12px; font-weight: 600; }
QLabel#pill[kind="white"] { background: #FFFFFF; color: #0B1C30; border-color: #E4E8F1;  border-radius: 12px; }
QLabel#pill[kind="tint"]  { background: #E1EAFF; color: #1D4ED8; border-color: #E1EAFF;  border-radius: 12px; }
QLabel#pill[kind="green"] { background: #E3F6EC; color: #0B7A4B; border-color: #E3F6EC;  border-radius: 12px; }
QLabel#pill[kind="red"]   { background: #FFDAD6; color: #93000A; border-color: #FFDAD6;  border-radius: 12px; }
QLabel#pill[kind="idle"]  { background: #EEF2FA; color: #5A6075; border-color: #EEF2FA;  border-radius: 12px; }
QLabel#monoPill { background: #FFFFFF; border: 1px solid #E4E8F1; border-radius: 8px; padding: 6px 12px;
                  font-family: "JetBrains Mono","Consolas","DejaVu Sans Mono",monospace; font-size: 12px; }

/* ---------- buttons ---------- */
QPushButton#btn { border-radius: 8px; padding: 0 16px; min-height: 38px; font-size: 14px; font-weight: 600; outline: none; }
QPushButton#btn[kind="primary"] { background: #2563EB; color: #FFFFFF; border: none; }
QPushButton#btn[kind="primary"]:hover   { background: #1D4ED8; }
QPushButton#btn[kind="primary"]:pressed { background: #1E40AF; }
QPushButton#btn[kind="primary"]:disabled{ background: #B9CCF8; color: #F4F7FF; }
QPushButton#btn[kind="deep"] { background: #0B4BCB; color: #FFFFFF; border: none; }
QPushButton#btn[kind="deep"]:hover   { background: #003EA8; }
QPushButton#btn[kind="deep"]:pressed { background: #00338A; }
QPushButton#btn[kind="deep"]:disabled{ background: #B3C3E8; color: #F4F7FF; }
QPushButton#btn[kind="outline"] { background: #FFFFFF; color: #334155; border: 1px solid #E1E6F0; }
QPushButton#btn[kind="outline"]:hover    { background: #F8FAFF; border-color: #C9D3E6; }
QPushButton#btn[kind="outline"]:disabled { color: #A3AEC0; background: #FAFBFD; }
QPushButton#btn[kind="tint"] { background: #E1EAFF; color: #1D4ED8; border: none; }
QPushButton#btn[kind="tint"]:hover    { background: #D3E0FD; }
QPushButton#btn[kind="tint"]:disabled { background: #EEF2FA; color: #A3AEC0; }
QPushButton#btn[kind="soft"] { background: #EAF0FD; color: #1F2A44; border: none; }
QPushButton#btn[kind="soft"]:hover { background: #DCE6FA; }
QPushButton#btn[kind="ghost"] { background: #FFFFFF; color: #334155; border: 1px solid #EEF1F7; }
QPushButton#btn[kind="ghost"]:hover { background: #F4F7FD; }
QPushButton#btn[kind="link"]  { background: transparent; color: #004AC6; border: none; padding: 0 6px; min-height: 26px; font-size: 13px; }
QPushButton#btn[kind="link"]:hover { background: #E8EFFD; }
QPushButton#btn[kind="danger"]{ background: transparent; color: #BA1A1A; border: none; padding: 0 6px; min-height: 26px; font-size: 13px; }
QPushButton#btn[kind="danger"]:hover { background: #FFEDEB; }
QPushButton#btn[kind="square"] { background: #FFFFFF; color: #334155; border: 1px solid #E1E6F0; padding: 0; min-width: 38px; max-width: 38px; }
QPushButton#btn[kind="square"]:hover { background: #F8FAFF; }

QPushButton#chipBtn { background: #E8EFFD; border: none; border-radius: 8px; padding: 0 14px; min-height: 32px;
                      font-family: "JetBrains Mono","Consolas","DejaVu Sans Mono",monospace; font-size: 12px; color: #1F2A44; text-align: left; }
QPushButton#chipBtn:hover { background: #DAE5FB; }

QPushButton#tile { background: #FFFFFF; border: 1px solid #E4E8F1; border-radius: 10px; text-align: left; }
QPushButton#tile:hover { background: #F4F8FF; }
QPushButton#tile[active="true"] { background: #E5EEFF; border: 1px solid #C9D9F8; }

/* segmented tab control */
QFrame#segBar { background: #E5EEFF; border: none; border-radius: 10px; }
QFrame#segBar[small="true"] { border-radius: 8px; }
QPushButton#seg { background: transparent; border: none; border-radius: 8px; padding: 0 14px; min-height: 36px;
                  color: #434655; font-size: 14px; font-weight: 500; outline: none; }
QPushButton#seg:hover   { color: #0B1C30; }
QPushButton#seg:checked { background: #FFFFFF; color: #004AC6; font-weight: 600; }
QFrame#segBar[small="true"] QPushButton#seg { min-height: 28px; padding: 0 10px; font-size: 12px; border-radius: 6px; }

/* ---------- inputs ---------- */
QLineEdit#codeInput { background: #FFFFFF; border: 1px solid #D5DBE8; border-radius: 8px; padding: 0 12px; min-height: 48px;
                      font-size: 20px; font-weight: 500; selection-background-color: #BFD3FB; }
QLineEdit#codeInput:hover { border-color: #B9C3D8; }
QLineEdit#codeInput:focus { border: 2px solid #2563EB; padding: 0 11px; }
QLineEdit#codeInput[state="error"] { border: 2px solid #DC2626; padding: 0 11px; }

QComboBox { background: #FFFFFF; border: 1px solid #E1E6F0; border-radius: 8px; padding: 0 12px; min-height: 36px; min-width: 120px; font-size: 13px; }
QComboBox:hover { border-color: #C3CCDF; }
QComboBox::drop-down { border: none; width: 26px; }
QComboBox QAbstractItemView { background: #FFFFFF; border: 1px solid #CBD5E1; selection-background-color: #EFF4FF;
                              selection-color: #004AC6; outline: none; padding: 4px; }

QSlider::groove:horizontal { height: 4px; background: #D3DCEE; border-radius: 2px; }
QSlider::sub-page:horizontal { background: #2563EB; border-radius: 2px; }
QSlider::handle:horizontal { background: #2563EB; border: 3px solid #FFFFFF; width: 10px; height: 10px; margin: -8px 0; border-radius: 8px; }

QProgressBar { background: #E1E9F8; border: none; border-radius: 2px; max-height: 4px; min-height: 4px; text-align: center; }
QProgressBar::chunk { background: #0B4BCB; border-radius: 2px; }

QScrollArea { border: none; background: transparent; }
QScrollBar:vertical { background: transparent; width: 10px; margin: 2px; }
QScrollBar::handle:vertical { background: #CBD5E8; border-radius: 4px; min-height: 30px; }
QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0; }
)");
}

inline QString nightModeStyleSheet()
{
    return QStringLiteral(R"(
* { color: #F8FAFC; }
QWidget#root, QWidget#page, QStackedWidget { background: #0B0F19; }
QToolTip { background: #1E293B; color: #F8FAFC; border: 1px solid #334155; padding: 5px 8px; border-radius: 6px; }

/* ---------- chrome ---------- */
QFrame#sidebar { background: #0F172A; border: none; border-right: 1px solid #1E293B; }
QFrame#topbar  { background: #0F172A; border: none; border-bottom: 1px solid #1E293B; }
QFrame#brandRow{ background: transparent; border: none; border-bottom: 1px solid #1E293B; }

QPushButton#nav { text-align: left; padding: 0 12px; min-height: 40px; max-height: 40px; border: 1px solid transparent;
                  border-radius: 8px; color: #94A3B8; font-size: 14px; font-weight: 500; background: transparent; }
QPushButton#nav:hover   { background: #1E293B; color: #F8FAFC; }
QPushButton#nav:checked { background: #1E293B; border: 1px solid #2563EB; color: #60A5FA; font-weight: 600; }

/* ---------- surfaces ---------- */
QFrame#card   { background: #111827; border: 1px solid #1F2937; border-radius: 12px; }
QFrame#panel  { background: #162032; border: 1px solid #243047; border-radius: 10px; }
QFrame#panelB { background: #162032; border: 1px solid #243047; border-radius: 10px; }
QFrame#white  { background: #162032; border: 1px solid #243047; border-radius: 10px; }
QFrame#tileSel{ background: #1E293B; border: 1px solid #2563EB; border-radius: 10px; }
QFrame#fail   { background: #3B1219; border: 1px solid #7F1D1D; border-radius: 12px; }
QFrame#hline  { background: #1F2937; border: none; min-height: 1px; max-height: 1px; }
QFrame#iconTile { background: #1E293B; border: none; border-radius: 10px; }
QFrame#avatar { background: #2563EB; border: none; border-radius: 20px; }

/* ---------- chips & pills ---------- */
QLabel#chip { border: 1px solid transparent; border-radius: 6px; padding: 2px 8px; font-family: "JetBrains Mono","Consolas","DejaVu Sans Mono",monospace;
              font-size: 11px; font-weight: 600; }
QLabel#chip[kind="blue"]    { background: #1E2D4A; color: #60A5FA; border-color: #1E2D4A; border-radius: 6px; }
QLabel#chip[kind="neutral"] { background: #1E293B; color: #CBD5E1; border-color: #334155; border-radius: 6px; }
QLabel#chip[kind="white"]   { background: #1E293B; color: #E2E8F0; border-color: #334155; border-radius: 6px; }
QLabel#chip[kind="green"]   { background: #064E3B; color: #34D399; border-color: #064E3B; border-radius: 6px; }

QLabel#pill { border: 1px solid transparent; border-radius: 12px; padding: 3px 11px; font-size: 12px; font-weight: 600; }
QLabel#pill[kind="white"] { background: #162032; color: #F8FAFC; border-color: #334155; border-radius: 12px; }
QLabel#pill[kind="tint"]  { background: #1E2D4A; color: #60A5FA; border-color: #1E2D4A; border-radius: 12px; }
QLabel#pill[kind="green"] { background: #064E3B; color: #34D399; border-color: #064E3B; border-radius: 12px; }
QLabel#pill[kind="red"]   { background: #7F1D1D; color: #FCA5A5; border-color: #7F1D1D; border-radius: 12px; }
QLabel#pill[kind="idle"]  { background: #1E293B; color: #94A3B8; border-color: #334155; border-radius: 12px; }
QLabel#monoPill { background: #162032; border: 1px solid #334155; border-radius: 8px; padding: 6px 12px;
                  color: #E2E8F0; font-family: "JetBrains Mono","Consolas","DejaVu Sans Mono",monospace; font-size: 12px; }

/* ---------- buttons ---------- */
QPushButton#btn { border-radius: 8px; padding: 0 16px; min-height: 38px; font-size: 14px; font-weight: 600; outline: none; }
QPushButton#btn[kind="primary"] { background: #2563EB; color: #FFFFFF; border: none; }
QPushButton#btn[kind="primary"]:hover   { background: #1D4ED8; }
QPushButton#btn[kind="primary"]:pressed { background: #1E40AF; }
QPushButton#btn[kind="primary"]:disabled{ background: #1E293B; color: #64748B; }
QPushButton#btn[kind="deep"] { background: #1D4ED8; color: #FFFFFF; border: none; }
QPushButton#btn[kind="deep"]:hover   { background: #1E40AF; }
QPushButton#btn[kind="deep"]:pressed { background: #172554; }
QPushButton#btn[kind="deep"]:disabled{ background: #1E293B; color: #64748B; }
QPushButton#btn[kind="outline"] { background: #162032; color: #E2E8F0; border: 1px solid #334155; }
QPushButton#btn[kind="outline"]:hover    { background: #1E293B; border-color: #475569; }
QPushButton#btn[kind="outline"]:disabled { color: #64748B; background: #0F172A; border-color: #1E293B; }
QPushButton#btn[kind="tint"] { background: #1E2D4A; color: #60A5FA; border: none; }
QPushButton#btn[kind="tint"]:hover    { background: #26385C; }
QPushButton#btn[kind="tint"]:disabled { background: #1E293B; color: #64748B; }
QPushButton#btn[kind="soft"] { background: #162032; color: #E2E8F0; border: 1px solid #243047; }
QPushButton#btn[kind="soft"]:hover { background: #1E293B; }
QPushButton#btn[kind="ghost"] { background: #111827; color: #CBD5E1; border: 1px solid #334155; }
QPushButton#btn[kind="ghost"]:hover { background: #162032; }
QPushButton#btn[kind="link"]  { background: transparent; color: #60A5FA; border: none; padding: 0 6px; min-height: 26px; font-size: 13px; }
QPushButton#btn[kind="link"]:hover { background: #1E293B; }
QPushButton#btn[kind="danger"]{ background: transparent; color: #EF4444; border: none; padding: 0 6px; min-height: 26px; font-size: 13px; }
QPushButton#btn[kind="danger"]:hover { background: #3B1219; }
QPushButton#btn[kind="square"] { background: #162032; color: #E2E8F0; border: 1px solid #334155; padding: 0; min-width: 38px; max-width: 38px; }
QPushButton#btn[kind="square"]:hover { background: #1E293B; }

QPushButton#chipBtn { background: #162032; border: 1px solid #243047; border-radius: 8px; padding: 0 14px; min-height: 32px;
                      font-family: "JetBrains Mono","Consolas","DejaVu Sans Mono",monospace; font-size: 12px; color: #E2E8F0; text-align: left; }
QPushButton#chipBtn:hover { background: #1E293B; }

QPushButton#tile { background: #111827; border: 1px solid #1F2937; border-radius: 10px; text-align: left; }
QPushButton#tile:hover { background: #162032; }
QPushButton#tile[active="true"] { background: #16233B; border: 1px solid #2563EB; }

/* segmented tab control */
QFrame#segBar { background: #162032; border: 1px solid #243047; border-radius: 10px; }
QFrame#segBar[small="true"] { border-radius: 8px; }
QPushButton#seg { background: transparent; border: none; border-radius: 8px; padding: 0 14px; min-height: 36px;
                  color: #94A3B8; font-size: 14px; font-weight: 500; outline: none; }
QPushButton#seg:hover   { color: #F8FAFC; }
QPushButton#seg:checked { background: #2563EB; color: #FFFFFF; font-weight: 600; }
QFrame#segBar[small="true"] QPushButton#seg { min-height: 28px; padding: 0 10px; font-size: 12px; border-radius: 6px; }

/* ---------- inputs ---------- */
QLineEdit#codeInput { background: #0B1120; color: #F8FAFC; border: 1px solid #334155; border-radius: 8px; padding: 0 12px; min-height: 48px;
                      font-size: 20px; font-weight: 500; selection-background-color: #1E3A8A; }
QLineEdit#codeInput:hover { border-color: #475569; }
QLineEdit#codeInput:focus { border: 2px solid #3B82F6; padding: 0 11px; }
QLineEdit#codeInput[state="error"] { border: 2px solid #EF4444; padding: 0 11px; }

QComboBox { background: #162032; color: #F8FAFC; border: 1px solid #334155; border-radius: 8px; padding: 0 12px; min-height: 36px; min-width: 120px; font-size: 13px; }
QComboBox:hover { border-color: #475569; }
QComboBox::drop-down { border: none; width: 26px; }
QComboBox QAbstractItemView { background: #111827; color: #F8FAFC; border: 1px solid #334155; selection-background-color: #1E293B;
                              selection-color: #60A5FA; outline: none; padding: 4px; }

QSlider::groove:horizontal { height: 4px; background: #334155; border-radius: 2px; }
QSlider::sub-page:horizontal { background: #3B82F6; border-radius: 2px; }
QSlider::handle:horizontal { background: #3B82F6; border: 3px solid #111827; width: 10px; height: 10px; margin: -8px 0; border-radius: 8px; }

QProgressBar { background: #1E293B; border: none; border-radius: 2px; max-height: 4px; min-height: 4px; text-align: center; }
QProgressBar::chunk { background: #3B82F6; border-radius: 2px; }

QScrollArea { border: none; background: transparent; }
QScrollArea > QWidget > QWidget { background: transparent; }
QScrollBar:vertical { background: transparent; width: 10px; margin: 2px; }
QScrollBar::handle:vertical { background: #334155; border-radius: 4px; min-height: 30px; }
QScrollBar::handle:vertical:hover { background: #475569; }
QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0; }

QTableWidget#scanHistory { background: #0B1120; color: #F8FAFC; gridline-color: #1E293B; border: 1px solid #1E293B; border-radius: 8px; }
QHeaderView::section { background: #162032; color: #94A3B8; border: none; border-bottom: 1px solid #1E293B; padding: 6px; }
)");
}

// Dedicated Night Mode toggle function.
// Can be called from any part of the application without altering other developers' code.
inline void setNightMode(bool enable, QWidget *window = nullptr)
{
    isNightMode() = enable;
    if (qApp) {
        qApp->setStyleSheet(enable ? nightModeStyleSheet() : styleSheet());
    }

    if (!window && qApp) {
        const auto topLevels = QApplication::topLevelWidgets();
        if (!topLevels.isEmpty()) window = topLevels.first();
    }
    if (window) {
        // Automatically adapt inline-colored QLabels across the UI without needing any code edits in child pages
        const auto labels = window->findChildren<QLabel *>();
        for (QLabel *l : labels) {
            if (enable) {
                if (!l->property("dayStyle").isValid()) {
                    l->setProperty("dayStyle", l->styleSheet());
                }
                QString s = l->property("dayStyle").toString();
                s.replace("#0B1C30", "#F8FAFC");
                s.replace("#434655", "#94A3B8");
                s.replace("#1F2A44", "#E2E8F0");
                s.replace("#737686", "#64748B");
                s.replace("#1F4FCF", "#60A5FA");
                s.replace("#0B4BCB", "#3B82F6");
                s.replace("#004AC6", "#60A5FA");
                l->setStyleSheet(s);

                const QString txt = l->text();
                if (txt.contains("#0B1C30") || txt.contains("#434655") || txt.contains("#1F2A44")) {
                    if (!l->property("dayText").isValid()) {
                        l->setProperty("dayText", txt);
                    }
                    QString t = l->property("dayText").toString();
                    t.replace("#0B1C30", "#F8FAFC");
                    t.replace("#434655", "#94A3B8");
                    t.replace("#1F2A44", "#E2E8F0");
                    l->setText(t);
                }
            } else {
                if (l->property("dayStyle").isValid()) {
                    l->setStyleSheet(l->property("dayStyle").toString());
                }
                if (l->property("dayText").isValid()) {
                    l->setText(l->property("dayText").toString());
                }
            }
        }

        const auto frames = window->findChildren<QFrame *>();
        for (QFrame *f : frames) {
            if (f->objectName() == "iconChip" || f->objectName() == "iconTile") {
                if (enable) {
                    if (!f->property("dayStyle").isValid()) {
                        f->setProperty("dayStyle", f->styleSheet());
                    }
                    QString s = f->property("dayStyle").toString();
                    s.replace("#EFF4FF", "#1E293B");
                    s.replace("#E5EEFF", "#1E293B");
                    s.replace("#E1E8F7", "#334155");
                    s.replace("#D6E1F7", "#334155");
                    s.replace("#434655", "#E2E8F0");
                    f->setStyleSheet(s);
                } else {
                    if (f->property("dayStyle").isValid()) {
                        f->setStyleSheet(f->property("dayStyle").toString());
                    }
                }
            }
        }

        window->update();
    }
}

inline void toggleNightMode(QWidget *window = nullptr)
{
    setNightMode(!isNightMode(), window);
}

} // namespace Theme
