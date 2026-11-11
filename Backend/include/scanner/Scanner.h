#pragma once
#include "Decoder.h"
#include <opencv2/videoio.hpp>
#include <string>
class Scanner
{
    cv::VideoCapture camera;
    Decoder decoder;
    int index;
    int width;
    int height;
    std::string errorMsg;
    bool find(const cv::Mat &gray, cv::Rect &box) const;

public:
    Scanner(int idx = 0, int width = 640, int height = 480);
    // opens camera
    bool openCamera();
    // scans barcdoe located in find()
    bool scan(std::string &six);
    bool frameRead(std::string &six, cv::Rect &box, const cv::Mat &frame);
    std::string errMessage() const;
    ~Scanner();
};
