#pragma once
#include <QWidget>

class QLabel;
class QLineEdit;
class QPushButton;
class QSlider;
class QAction;
class QVBoxLayout;
class QHBoxLayout;
class BarcodeView;
class ToggleSwitch;
class RadioCard;
class SegBar;

// Screen 1 – "Generate Barcode" (6-digit code -> barcode).
class GeneratePage : public QWidget {
    Q_OBJECT
public:
    explicit GeneratePage(QWidget *parent = nullptr);

    void setCode(const QString &code, bool generate = true);   // e.g. from the Decode page
    void setBarcodeImage(const QPixmap &pm);                   // hook: show the real barcode

signals:
    void generateRequested(const QString &code);               // hook: user pressed Generate

private slots:
    void onTextChanged(const QString &t);
    void onGenerate();
    void onClear();
    void onSave();
    void onCopy();

private:
    struct Session { QString code, symbology, time; };
    void rebuildSessions();

    QLineEdit *m_input;
    QAction *m_checkAction;
    QLabel *m_counter, *m_hint, *m_hintIcon, *m_barVal, *m_quietVal;
    QWidget *m_validChip;
    QPushButton *m_generate, *m_clear, *m_save, *m_copy;
    RadioCard *m_code128, *m_ean8;
    QSlider *m_barSlider, *m_quietSlider;
    ToggleSwitch *m_ocr;
    BarcodeView *m_view;
    SegBar *m_stageSeg;
    QHBoxLayout *m_sessionRow;
    QList<Session> m_sessions;
    int m_activeSession = 0;
};
