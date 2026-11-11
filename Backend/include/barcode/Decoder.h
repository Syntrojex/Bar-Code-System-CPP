#pragma once
#include <opencv2/core.hpp>
#include <string>
#include <vector>
class Decoder
{
    std::vector<double> scanLineRuns(const cv::Mat &barcode) const;
    
    std::string runsToModules(const std::vector<double> &runs) const;
    
    static int digitMatch(const std::string &seven, bool right);
    
    std::string modulesToDigits(const std::string &bits) const;
    
    bool isCheckDigit(const std::string &d) const;
    
    static constexpr int stripes = 43;
    
    static constexpr int modules = 67;

    static const char *const patterns[10];
    
    mutable bool badCheckDigit = false;

public:
    bool decodeSix(const std::string &bits, std::string &six) const;

    bool decodeCropping(const cv::Mat &gray, std::string &six) const;

    bool isBadCheck() const;
};
