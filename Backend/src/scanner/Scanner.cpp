#include "Scanner.h"
#include <opencv2/imgproc.hpp>
using namespace std;
Scanner::Scanner(int idx, int width, int height)
{
    index = idx;
    this->width = width;
    this->height = height;
}
// Open the camera
bool Scanner::openCamera()
{
#ifdef _WIN32
    camera.open(index, cv::CAP_DSHOW);
#else
    camera.open(index);
#endif
    if (!camera.isOpened())
    {
        errorMsg = "Camera cannot open";
        return false;
    }
    // produces an MJPG of 640x480 screen
    camera.set(cv::CAP_PROP_FOURCC, cv::VideoWriter::fourcc('M', 'J', 'P', 'G'));
    camera.set(cv::CAP_PROP_FRAME_WIDTH, width);
    camera.set(cv::CAP_PROP_FRAME_HEIGHT, height);
    camera.set(cv::CAP_PROP_FPS, 30);
    return true;
}
// locate the barcode
bool Scanner::find(const cv::Mat &gray, cv::Rect &box) const
{
    cv::Mat gx, gy, score, mask;
    cv::Sobel(gray, gx, CV_32F, 1, 0, 3);
    cv::Sobel(gray, gy, CV_32F, 0, 1, 3);
    score = cv::max(cv::abs(gx) - cv::abs(gy), 0.0);
    score.convertTo(score, CV_8U, 0.25);
    //find the blob
    cv::blur(score, score, cv::Size(9, 9));
    cv::threshold(score, mask, 0, 255, cv::THRESH_BINARY | cv::THRESH_OTSU);
    cv::Mat k = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(21, 7));
    cv::morphologyEx(mask, mask, cv::MORPH_CLOSE, k);
    cv::erode(mask, mask, cv::Mat(), cv::Point(-1, -1), 4);
    cv::dilate(mask, mask, cv::Mat(), cv::Point(-1, -1), 4);
    vector<vector<cv::Point>> contours;
    cv::findContours(mask, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
    cv::Rect Best;
    double bestArea = 0;
    for (const vector<cv::Point> &c : contours)
    {
        double a = cv::contourArea(c);
        if (a > bestArea)
        {
            bestArea = a;
            Best = cv::boundingRect(c);
        }
    }
    if (bestArea > 2000 && Best.width > Best.height && Best.height > 40)
    {
        box = Best;
        return true;
    }
    return false;
}
// read each frame and locate the barcode in grayscale
bool Scanner::frameRead(std::string &six, cv::Rect &box, const cv::Mat &frame)
{
    box = cv::Rect();
    six.clear();
    errorMsg.clear();
    if (frame.empty())
    {
        errorMsg = "Camera stopped sending frames";
        return false;
    }
    cv::Mat gray;
    if (frame.channels() == 1)
        gray = frame;
    else if (frame.channels() == 3)
        cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);
    else if (frame.channels() == 4)
        cv::cvtColor(frame, gray, cv::COLOR_BGRA2GRAY);
    else
        return false;
    // Decode across several scan lines before relying on the contour detector.
    if (decoder.decodeCropping(gray, six))
    {
        cv::Rect detected;
        box = find(gray, detected) ? detected : cv::Rect(0, 0, gray.cols, gray.rows);
        return true;
    }
    cv::Rect Best;
    if (!find(gray, Best))
    {
        cv::Mat rotated;
        cv::rotate(gray, rotated, cv::ROTATE_90_CLOCKWISE);
        if (decoder.decodeCropping(rotated, six))
        {
            box = cv::Rect(0, 0, gray.cols, gray.rows);
            return true;
        }
        return false;
    }
    box = Best;
    cv::Rect pad(Best.x - 15, Best.y, Best.width + 30, Best.height);
    pad &= cv::Rect(0, 0, gray.cols, gray.rows);
    cv::Mat barcode = gray(pad).clone();
    return decoder.decodeCropping(barcode, six);
}
// scanning the located barcode for the six digits
bool Scanner::scan(std::string &sixDigits)
{
    sixDigits.clear();
    if (!camera.isOpened() && !openCamera())
        return false;
    const auto start = cv::getTickCount();
    // tick count
    while ((cv::getTickCount() - start) / cv::getTickFrequency() < 30.0)
    {
        cv::Mat frame;
        if (!camera.read(frame) || frame.empty())
        {
            errorMsg = "Camera stopped sending frames";
            camera.release();
            return false;
        }
        cv::Rect box;
        if (frameRead(sixDigits, box, frame))
        {
            camera.release();
            return true;
        }
    }
    camera.release();
    errorMsg = "No barcode found within 30 seconds";
    return false;
}
// store the error message
std::string Scanner::errMessage() const
{
    return errorMsg;
}
// close the camera
Scanner::~Scanner()
{
    camera.release();
}
