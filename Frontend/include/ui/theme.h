#pragma once
#include <QString>

// Design tokens + global stylesheet, taken from the Stitch "Precision Utility" export.
// NOTE: font family/size are NOT set here (QSS would override setFont) – see main.cpp.
namespace Theme {

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

} // namespace Theme
