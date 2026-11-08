#include "scanner/CameraScanner.h"

bool CameraScanner::openCamera(int cameraIndex, QString *error)
{
#ifdef HAVE_OPENCV
    if (capture.open(cameraIndex)) {
        return true;
    }
    if (error) *error = "Failed to open camera";
    return false;
#else
    Q_UNUSED(cameraIndex);
    Q_UNUSED(error);
    m_opened = true;
    return true;
#endif
}

#ifdef HAVE_OPENCV
#include <opencv2/imgproc.hpp>
#endif

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
        cv::Mat rgb;
        cv::cvtColor(mat, rgb, cv::COLOR_BGR2RGB);
        res.frame = QImage(rgb.data, rgb.cols, rgb.rows, static_cast<int>(rgb.step), QImage::Format_RGB888).copy();
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
