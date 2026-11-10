#pragma once

#include <QString>
#include <QImage>
#include <QRect>
#include <QStringList>

#ifdef HAVE_OPENCV
#include <memory>
#include <opencv2/core.hpp>
#include <opencv2/videoio.hpp>
class Scanner;
#endif

struct CameraFrameResult
{
    bool success = false;
    QImage frame;
    QString decodedText;   // 6-digit code when a barcode was decoded in this frame (OpenCV builds)
    QRect box;             // barcode location in frame coordinates (empty when none was found)
    int decodeMs = 0;      // time spent searching/decoding this frame
    QString error;
};

class CameraScanner
{
public:
    CameraScanner();
    ~CameraScanner();
    CameraScanner(const CameraScanner &) = delete;
    CameraScanner &operator=(const CameraScanner &) = delete;

    static QStringList availableCameras();
    bool openCamera(int cameraIndex = 0, QString *error = nullptr, QSize resolution = QSize(1280, 720));
    CameraFrameResult readFrame(bool decode = true);
    bool autofocus();
    void closeCamera();
    bool isOpened() const;

private:
#ifdef HAVE_OPENCV
    cv::VideoCapture capture;
    std::unique_ptr<Scanner> m_decoder;             // frame decoder from Backend/Scanner
#else
    bool m_opened = false;
#endif
};
