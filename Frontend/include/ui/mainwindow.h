#pragma once
#include <QMainWindow>
#include <QString>
#include <memory>

class QButtonGroup;
class QStackedWidget;
class GeneratePage;
class DecodePage;
class CameraScanner;
class QTimer;
class QTemporaryDir;

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    GeneratePage *generatePage() const { return m_generate; }
    DecodePage   *decodePage()   const { return m_decode; }

    ~MainWindow() override;

private:
    void startCamera(int index);
    void stopCamera();
    void pumpCamera();
    void decodeFile(const QString &path);           // Upload tab / samples: ImageScanner -> DecodePage
    void makeSample(const QString &code, bool damaged);
    QWidget *buildSidebar();
    QWidget *buildTopBar();

    QStackedWidget *m_pages;
    QButtonGroup *m_nav;
    GeneratePage *m_generate;
    DecodePage *m_decode;
    CameraScanner *m_scanner = nullptr;
    QTimer *m_camTimer = nullptr;
    int m_failedReads = 0;
    int m_missFrames = 0;
    QString m_lastCode;                             // last code shown from the camera (avoid re-announcing it every frame)
    std::unique_ptr<QTemporaryDir> m_tmp;           // generated sample images
};
