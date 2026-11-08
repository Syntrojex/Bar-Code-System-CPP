#pragma once
#include <QWidget>
#include <QStringList>

class QComboBox;
class QLabel;
class QPushButton;
class QStackedWidget;
class QFrame;
class CameraView;
class DropZone;
class SegBar;

// Screens 2-4 – "Decode Barcode": camera tab, upload tab and the "Scan Logs" overview.
class DecodePage : public QWidget {
    Q_OBJECT
public:
    explicit DecodePage(QWidget *parent = nullptr);

    void setTab(int index);                         // 0 camera, 1 upload, 2 scan logs

    // ---- hooks for the backend ----
    void setCameraList(const QStringList &names);
    CameraView *cameraView() const { return m_camera; }
    void showResult(const QString &code);           // success state on every screen
    // "Could not read barcode"; reason / errCode are optional (defaults keep old calls working)
    void showFailure(const QString &reason = QString(), const QString &errCode = QString());
    void resetResult();

signals:
    void cameraStartRequested(int cameraIndex);
    void cameraStopRequested();
    void decodeImageRequested(const QString &path);
    void generateFromCodeRequested(const QString &code);

private:
    struct ResultUi {
        QLabel *badge = nullptr;
        QLabel *code = nullptr;
        QPushButton *copy = nullptr;
        QFrame *fail = nullptr;
        bool detectedWording = false;               // "Barcode Detected" instead of "Success"
        const char *okKind = "tint";
    };

    QWidget *buildCameraPage();
    QWidget *buildUploadPage();
    QWidget *buildLogsPage();
    SegBar *makeTabs(int current);
    QFrame *makeFailBox();
    void applyResult(const ResultUi &r, const QString &code);
    void loadImage(const QString &path);

    QStackedWidget *m_stack;
    // camera
    QComboBox *m_cameraBox = nullptr;
    QPushButton *m_stream = nullptr, *m_freeze = nullptr;
    CameraView *m_camera = nullptr;
    bool m_streaming = true;
    // upload
    QFrame *m_loaded = nullptr;
    QLabel *m_thumb = nullptr, *m_fileName = nullptr, *m_fileMeta = nullptr;
    QString m_path;
    QPushButton *m_simulate = nullptr;
    bool m_simulating = false;

    ResultUi m_cam, m_up, m_log;
};
