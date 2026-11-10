#include "Scanner.h"
#include <opencv2/imgproc.hpp>
using namespace std;
Scanner::Scanner(int idx, int width, int height)
{
    index = idx;
    this->width = width;
    this->height = height;
}
bool Scanner::openCamera()
{
    camera.open(0, cv::CAP_DSHOW);
    if (!camera.isOpened())
    {
        errorMsg = "Camera cannot open";
        return false;
    }
    camera.set(cv::CAP_PROP_FOURCC, cv::VideoWriter::fourcc('M', 'J', 'P', 'G'));
    camera.set(cv::CAP_PROP_FRAME_WIDTH, width);
    camera.set(cv::CAP_PROP_FRAME_HEIGHT, height);
    camera.set(cv::CAP_PROP_FPS, 30);
    return true;
}
bool Scanner::find(const cv::Mat &gray, cv::Rect &box) const
{
    cv::Mat gx, gy, score, mask;
    cv::Sobel(gray, gx, CV_32F, 1, 0, 3);
    cv::Sobel(gray, gy, CV_32F, 0, 1, 3);
    score = cv::max(cv::abs(gx) - cv::abs(gy), 0.0);
    score.convertTo(score, CV_8U, 0.25);
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
bool Scanner::frameRead(std::string &six, cv::Rect &box, const cv::Mat &frame)
{
    box = cv::Rect();
    if (frame.empty())
    {
        errorMsg = "Camera stopped sending frames";
        return false;
    }
    cv::Mat gray;
    cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);
    cv::Rect Best;
    if (!find(gray, Best))
        return false;
    box = Best;
    cv::Rect pad(Best.x - 15, Best.y, Best.width + 30, Best.height);
    pad &= cv::Rect(0, 0, gray.cols, gray.rows);
    cv::Mat barcode = gray(pad).clone();
    return decoder.decodeCropping(barcode, six);
}
bool Scanner::scan(std::string &sixDigits)
{
    while (true)
    {
        if (!camera.isOpened() && !openCamera())
            return false;
        cv::Mat frame;
        camera >> frame;
        if (frame.empty())
            return false;
        cv::Rect Best;
        string six;
        bool isDecoded = frameRead(six, Best, frame);
        cv::Mat view = frame.clone();
        if (Best.area() > 0)
        {
            cv::rectangle(view, Best, cv::Scalar(0, 255, 0), 2);
            cv::putText(view, isDecoded ? six : (decoder.isBadCheck() ? "Wrong check digit" : "searching..."),
                        cv::Point(Best.x, Best.y - 10), cv::FONT_HERSHEY_SIMPLEX, 0.9,
                        isDecoded ? cv::Scalar(255, 0, 0) : cv::Scalar(0, 0, 255), 2);
        }
        cv::imshow("live camera", view);
        int key = cv::waitKey(1);
        if (isDecoded)
        {
            sixDigits = six;
        }
        if (key == 27)
        {
            if (sixDigits.empty())
                errorMsg = "request cancelled";
            return !sixDigits.empty();
        }
    }
}
std::string Scanner::errMessage() const
{
    return errorMsg;
}
Scanner::~Scanner()
{
    camera.release();
    cv::destroyAllWindows();
}