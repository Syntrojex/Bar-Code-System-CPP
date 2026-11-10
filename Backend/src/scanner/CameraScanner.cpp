#include "scanner/CameraScanner.h"

#ifdef HAVE_OPENCV
#include <QElapsedTimer>
#include <opencv2/imgproc.hpp>
#include "Scanner.h"
#endif

CameraScanner::CameraScanner()
{
#ifdef HAVE_OPENCV
    m_decoder = std::make_unique<Scanner>();        // used only for frameRead(); it never opens its own camera here
#endif
}

CameraScanner::~CameraScanner() = default;

bool CameraScanner::openCamera(int cameraIndex, QString *error)
{
#ifdef HAVE_OPENCV
#ifdef _WIN32
    const bool opened = capture.open(cameraIndex, cv::CAP_DSHOW);   // DirectShow: fast start, same backend as Backend/Scanner
#else
    const bool opened = capture.open(cameraIndex);
#endif
    if (opened) {
        capture.set(cv::CAP_PROP_FRAME_WIDTH, 640);                 // small frames keep the decoder fast
        capture.set(cv::CAP_PROP_FRAME_HEIGHT, 480);
        return true;
    }
    if (error) *error = QString("Failed to open camera %1. Check that it is connected and not used by another app.").arg(cameraIndex);
    return false;
#else
    Q_UNUSED(cameraIndex);
    Q_UNUSED(error);
    m_opened = true;
    return true;
#endif
}

CameraFrameResult CameraScanner::readFrame()
{
    CameraFrameResult res;
#ifdef HAVE_OPENCV
    if (!capture.isOpened()) {
        res.error = "Camera not opened";
        return res;
    }
    cv::Mat mat;
    if (capture.read(mat) && !mat.empty()) {
        res.success = true;

        QElapsedTimer timer;
        timer.start();
        std::string six;
        cv::Rect box;
        if (m_decoder->frameRead(six, box, mat))
            res.decodedText = QString::fromStdString(six);
        if (box.area() > 0)
            res.box = QRect(box.x, box.y, box.width, box.height);
        res.decodeMs = static_cast<int>(timer.elapsed());

        cv::Mat rgb;
        cv::cvtColor(mat, rgb, cv::COLOR_BGR2RGB);
        res.frame = QImage(rgb.data, rgb.cols, rgb.rows, static_cast<int>(rgb.step), QImage::Format_RGB888).copy();
    } else {
        res.error = "Camera stopped delivering frames.";
    }
#else
    res.success = m_opened;
#endif
    return res;
}

void CameraScanner::closeCamera()
{
#ifdef HAVE_OPENCV
    if (capture.isOpened()) {
        capture.release();
    }
#else
    m_opened = false;
#endif
}

bool CameraScanner::isOpened() const
{
#ifdef HAVE_OPENCV
    return capture.isOpened();
#else
    return m_opened;
#endif
}
