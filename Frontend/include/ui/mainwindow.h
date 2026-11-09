#pragma once
#include <QMainWindow>

class QButtonGroup;
class QStackedWidget;
class GeneratePage;
class DecodePage;
class CameraScanner;
class QTimer;

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
    QWidget *buildSidebar();
    QWidget *buildTopBar();

    QStackedWidget *m_pages;
    QButtonGroup *m_nav;
    GeneratePage *m_generate;
    DecodePage *m_decode;
    CameraScanner *m_scanner = nullptr;
    QTimer *m_camTimer = nullptr;
    int m_failedReads = 0;
};
