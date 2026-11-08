#pragma once

#include <QString>
#include <QImage>

#ifdef HAVE_OPENCV
#include <opencv2/core.hpp>
#include <opencv2/videoio.hpp>
#endif

struct CameraFrameResult
{
    bool success = false;
    QImage frame;
    QString decodedText;
    QString error;
};

class CameraScanner
{
public:
    bool openCamera(int cameraIndex = 0, QString *error = nullptr);
    CameraFrameResult readFrame();
    void closeCamera();
    bool isOpened() const;

private:
#ifdef HAVE_OPENCV
    cv::VideoCapture capture;
#else
    bool m_opened = false;
#endif
};
