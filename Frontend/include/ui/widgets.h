#pragma once
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QPixmap>
#include <QPushButton>
#include <QTimer>
#include <QVBoxLayout>
#include <QWidget>
#include <QColor>
#include <QDragEnterEvent>
#include <QDragLeaveEvent>
#include <QDropEvent>
#include <QList>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QPainter>
#include <QSize>
#include <QString>
#include <QStringList>
#include <QFont>

// ======================================================= layout / factory helpers
QVBoxLayout *vbox(QWidget *w, int l, int t, int r, int b, int spacing);
QHBoxLayout *hbox(QWidget *w, int l, int t, int r, int b, int spacing);
QHBoxLayout *hrow(int spacing = 8);                 // free-standing layouts, zero margins
QVBoxLayout *vcol(int spacing = 8);

QString monoFamily();                               // CSS font-family list for mono text
QFont   monoFont(int px, bool bold = false);
QLabel *T(const QString &text, int px, int weight, const QString &color, bool mono = false);  // styled label
QLabel *caption(const QString &text, bool mono = false);                                       // small grey caps label
QLabel *chip(const QString &text, const char *kind = "blue");                                  // mono tag
QLabel *pill(const QString &text, const char *kind = "white");                                 // rounded status pill
QLabel *monoPill(const QString &text);
QFrame *card();
QFrame *panel(const char *objectName = "panel");
QFrame *hline();
QFrame *iconTile(const QString &icon, const QColor &color, int box = 40, int iconSize = 20);
QPushButton *btn(const QString &text, const char *kind, const QString &icon = QString());
void    setKind(QWidget *w, const char *kind);      // dynamic property + re-polish
void    setBadge(QLabel *l, const QString &text, const char *kind);

// ======================================================= mock barcode drawing
// UI-only decorative bars derived from the digits (NOT a real symbology).
void drawMockBars(QPainter &p, const QRect &r, const QString &code, const QColor &color);

class MiniBarcode : public QWidget {                // small barcode glyph (thumbnails, payload previews)
    Q_OBJECT
public:
    explicit MiniBarcode(const QString &code, QSize size, QWidget *parent = nullptr);
    void setCode(const QString &c) { m_code = c; update(); }
protected:
    void paintEvent(QPaintEvent *) override;
private:
    QString m_code;
};

// ======================================================= Generate page: print stage
class BarcodeView : public QWidget {
    Q_OBJECT
public:
    explicit BarcodeView(QWidget *parent = nullptr);
    void setCode(const QString &code);              // placeholder bars until a real image is supplied
    void setImage(const QPixmap &pm);               // call this with the real generated barcode
    void setShowDigits(bool on);
    void setAwaiting(bool on);                      // "Awaiting State" view
    void clear();
    bool hasBarcode() const { return !m_code.isEmpty() || !m_image.isNull(); }
    QPixmap renderBarcode() const;                  // hi-res pixmap for Save / Copy
    QSize sizeHint() const override { return {560, 340}; }
protected:
    void paintEvent(QPaintEvent *) override;
private:
    void paintBarcodeCard(QPainter &p, const QRect &r) const;
    QString m_code;
    QPixmap m_image;
    bool m_digits = true, m_awaiting = false;
};

// ======================================================= Decode page: camera viewport
class CameraView : public QWidget {
    Q_OBJECT
public:
    explicit CameraView(QWidget *parent = nullptr);
    void setRunning(bool on);                       // camera on/off (off clears the picture)
    void setFrozen(bool on);                        // keep the camera running but stop updating the picture
    bool frozen() const { return m_frozen; }
    void setFrame(const QPixmap &pm);               // live camera frame (ignored while frozen)
    QSize sizeHint() const override { return {620, 350}; }
protected:
    void paintEvent(QPaintEvent *) override;
private:
    bool m_running = false, m_frozen = false;
    qreal m_phase = 0;
    QPixmap m_frame;
    QTimer m_timer;
};

// ======================================================= Decode page: upload drop zone
class DropZone : public QFrame {
    Q_OBJECT
public:
    explicit DropZone(QWidget *parent = nullptr);
signals:
    void fileChosen(const QString &path);
protected:
    void paintEvent(QPaintEvent *) override;
    void mousePressEvent(QMouseEvent *) override;
    bool event(QEvent *e) override;
    void dragEnterEvent(QDragEnterEvent *) override;
    void dragLeaveEvent(QDragLeaveEvent *) override;
    void dropEvent(QDropEvent *) override;
private:
    bool m_hover = false;
};

// ======================================================= misc controls
class ToggleSwitch : public QWidget {
    Q_OBJECT
public:
    explicit ToggleSwitch(bool on = true, QWidget *parent = nullptr);
    bool isChecked() const { return m_on; }
    void setChecked(bool on);
    QSize sizeHint() const override { return {44, 24}; }
signals:
    void toggled(bool on);
protected:
    void paintEvent(QPaintEvent *) override;
    void mouseReleaseEvent(QMouseEvent *) override;
private:
    bool m_on;
};

class RadioCard : public QFrame {                   // "Code 128 / EAN-8" selectable card
    Q_OBJECT
public:
    RadioCard(const QString &title, const QString &sub, QWidget *parent = nullptr);
    void setSelected(bool on);
    bool selected() const { return m_sel; }
signals:
    void clicked();
protected:
    void paintEvent(QPaintEvent *) override;
    void mouseReleaseEvent(QMouseEvent *) override;
private:
    bool m_sel = false;
};

class SegBar : public QFrame {                      // segmented tab control
    Q_OBJECT
public:
    SegBar(const QStringList &labels, const QStringList &icons, int current, bool small = false, QWidget *parent = nullptr);
    int current() const { return m_current; }
    void setCurrent(int i);
signals:
    void currentChanged(int index);
private:
    void refreshIcons();
    QList<QPushButton *> m_btns;
    QStringList m_icons;
    int m_current = 0;
};

// icon + text capsule (used for header pills and chips with an icon)
QFrame *iconChip(const QString &icon, const QString &text, const QString &bg, const QString &fg,
                 const QString &border = QString(), bool mono = true, int px = 12, int radius = 8);
