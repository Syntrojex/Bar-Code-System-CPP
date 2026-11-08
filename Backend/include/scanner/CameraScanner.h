#pragma once

#include <QString>
#include <QImage>

#include <opencv2/core.hpp>
#include <opencv2/videoio.hpp>

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
    cv::VideoCapture capture;
};
