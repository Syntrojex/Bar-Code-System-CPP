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
class QTableWidget;
class QCheckBox;

// Screens 2-4 – "Decode Barcode": camera tab, upload tab and the "Scan Logs" overview.
// optional details shown next to a decoded result
struct ScanInfo {
    int latencyMs = -1;                             // decode time
    QSize box;                                      // barcode size found in the image/frame
    QString payload;                                // full EAN-8 payload (8 digits)
};

class DecodePage : public QWidget {
    Q_OBJECT
public:
    explicit DecodePage(QWidget *parent = nullptr);

    void setTab(int index);                         // 0 camera, 1 upload, 2 scan logs

    // ---- hooks for the backend ----
    void setCameraList(const QStringList &names);
    QSize cameraResolution() const;
    bool mirrorCamera() const;
    void setCameraActive(bool on);                  // sync buttons/viewport with the real camera state
    void setCameraMessage(const QString &text, bool error = false);   // hint line under the viewport
    bool cameraActive() const { return m_streaming; }
    void stopCamera();                              // stops (and emits cameraStopRequested) if running
    CameraView *cameraView() const { return m_camera; }
    using ScanInfo = ::ScanInfo;
    void showResult(const QString &code, const ScanInfo &info = ScanInfo());   // success state on every screen
    // "Could not read barcode"; reason / errCode are optional (defaults keep old calls working)
    void showFailure(const QString &reason = QString(), const QString &errCode = QString());
    void resetResult();
    void loadImage(const QString &path);            // load an image into the upload tab (shows file info, clears result)

signals:
    void cameraStartRequested(int cameraIndex);
    void cameraStopRequested();
    void cameraRefreshRequested();
    void autofocusRequested();
    void decodeImageRequested(const QString &path);
    void sampleRequested(const QString &code, bool damaged);   // "Load Sample": generate a test image of this code
    void generateFromCodeRequested(const QString &code);

private:
    struct ResultUi {
        QLabel *badge = nullptr;
        QLabel *code = nullptr;
        QPushButton *copy = nullptr;
        QFrame *fail = nullptr;
        QLabel *bounds = nullptr, *stamp = nullptr, *latency = nullptr, *payload = nullptr;   // details (may be null)
        bool detectedWording = false;               // "Barcode Detected" instead of "Success"
        const char *okKind = "tint";
    };

    QWidget *buildCameraPage();
    QWidget *buildUploadPage();
    QWidget *buildLogsPage();
    SegBar *makeTabs(int current);
    QFrame *makeFailBox();
    void applyResult(const ResultUi &r, const QString &code);
    void clearDetails(const ResultUi &r);
    void exportHistory();

    QStackedWidget *m_stack;
    // camera
    QComboBox *m_cameraBox = nullptr;
    QComboBox *m_resolutionBox = nullptr;
    QCheckBox *m_mirror = nullptr;
    QPushButton *m_refresh = nullptr, *m_focus = nullptr, *m_snapshot = nullptr;
    QLabel *m_engine = nullptr;
    QList<SegBar *> m_tabs;
    QTableWidget *m_history = nullptr;
    bool m_autoCopy = false;
    QPushButton *m_stream = nullptr, *m_freeze = nullptr;
    CameraView *m_camera = nullptr;
    QLabel *m_camHint = nullptr;
    bool m_streaming = false;
    // upload
    QFrame *m_loaded = nullptr;
    QLabel *m_thumb = nullptr, *m_fileName = nullptr, *m_fileMeta = nullptr;
    QString m_path;
    QPushButton *m_simulate = nullptr;
    bool m_simulating = false;

    QList<QWidget *> m_okOnly;                      // "checksum passed" labels: visible only after a successful decode
    ResultUi m_cam, m_up, m_log;
};
