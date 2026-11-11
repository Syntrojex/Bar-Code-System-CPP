#pragma once
#include <opencv2/core.hpp>
#include <string>
#include <vector>
class Decoder
{ // scan and store weight at each line
    std::vector<double> scanLineRuns(const cv::Mat &barcode) const;
    // convert weights to modules
    std::string runsToModules(const std::vector<double> &runs) const;
    // return digit if it matches the pattern
    static int digitMatch(const std::string &seven, bool right);
    // convert modules (1-4) to digits
    std::string modulesToDigits(const std::string &bits) const;
    // is the check digit valid?
    bool isCheckDigit(const std::string &d) const;
    static constexpr int stripes = 43;
    static constexpr int modules = 67;
    static const char *const patterns[10];
    mutable bool badCheckDigit = false;

public:
    // store the digits
    bool decodeSix(const std::string &bits, std::string &six) const;
    // calls the functions above
    bool decodeCropping(const cv::Mat &gray, std::string &six) const;
    bool isBadCheck() const;
};
