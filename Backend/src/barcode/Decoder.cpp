#include <iostream>
#include <cmath>
#include <opencv2/imgproc.hpp>
#include "Decoder.h"
using namespace std;
// patterns representing numbers 0-9
const char *const Decoder::patterns[10] = {"0001101", "0011001", "0010011", "0111101", "0100011",
                                           "0110001", "0101111", "0111011", "0110111", "0001011"};
// store weights at each line
std::vector<double> Decoder::scanLineRuns(const cv::Mat &barcode) const
{
    if (barcode.empty() || barcode.type() != CV_8UC1 || barcode.rows < 5)
    {
        return {};
    }
    int y0 = barcode.rows * 4 / 10;
    int y1 = barcode.rows * 6 / 10;
    cv::Mat line;
    cv::reduce(barcode.rowRange(y0, y1), line, 0, cv::REDUCE_AVG, CV_32F);
    double low, high;
    cv::minMaxLoc(line, &low, &high);
    float T = (float)((low + high) / 2);
    vector<double> edges;
    for (int i = 0; i + 1 < line.cols; i++)
    {
        float a = line.at<float>(0, i);
        float b = line.at<float>(0, i + 1);
        if ((a < T) != (b < T))
        {
            edges.push_back(i + (T - a) / (b - a));
        }
    }
    if (edges.size() < 2)
        return {};
    // add in the new vector
    vector<double> runs;
    for (size_t j = 0; j + 1 < edges.size(); j++)
    {
        runs.push_back(edges[j + 1] - edges[j]);
    }
    return runs;
}
// converting 43 stripes to modules (between 1 and 4)
std::string Decoder::runsToModules(const std::vector<double> &runs) const
{
    if (runs.size() != stripes)
        return "";
    double total = 0;
    for (double r : runs)
        total += r;
    double module = total / modules;
    string bits;
    for (size_t i = 0; i < runs.size(); i++)
    {
        int n = (int)lround(runs[i] / module);
        if (n < 1 || n > 4)
            return "";
        bits += string(n, i % 2 == 0 ? '1' : '0');
    }
    return (bits.size() == modules) ? bits : "";
}
// matching digit with the given pattern
int Decoder::digitMatch(const std::string &seven, bool right)
{
    for (int i = 0; i < 10; i++)
    {
        string pattern = patterns[i];
        if (right)
        {
            for (char &c : pattern)
                c = (c == '1') ? '0' : '1';
        }
        if (pattern == seven)
            return i;
    }
    return -1;
}
// converting modules to digits
std::string Decoder::modulesToDigits(const std::string &bits) const
{
    if (bits.size() != modules || bits.substr(0, 3) != "101" || bits.substr(64, 3) != "101" || bits.substr(31, 5) != "01010")
        return "";
    string digits;
    // left slot of barcode
    for (int i = 0; i < 4; i++)
    {
        int d = digitMatch(bits.substr(3 + 7 * i, 7), false);
        if (d < 0)
            return "";
        digits += char('0' + d);
    }
    // right slot of barcode
    for (int j = 0; j < 4; j++)
    {
        int d = digitMatch(bits.substr(36 + 7 * j, 7), true);
        if (d < 0)
            return "";
        digits += char('0' + d);
    }
    return digits;
}
// is the check digit valid or not?
bool Decoder::isCheckDigit(const std::string &d) const
{ // 0482915 7
    int sum = 0;
    for (int i = 0; i < 7; i++)
        sum += (d[i] - '0') * ((i % 2 == 0) ? 3 : 1);
    // eg 73 -> 80 80-73 =7
    return (10 - sum % 10) % 10 == d[7] - '0';
}
// store the six digits
bool Decoder::decodeSix(const std::string &bits, std::string &six) const
{
    six.clear();
    badCheckDigit = false;
    for (int a = 0; a < 2; a++)
    {
        string b = (a == 0) ? bits : string(bits.rbegin(), bits.rend());
        string d = modulesToDigits(b);
        if (d.size() != 8)
            continue;
        if (!isCheckDigit(d))
        {
            badCheckDigit = true;
            continue;
        }
        if (d[0] == '0' && d[1] != '0') // 04829157
        {
            six = d.substr(1, 6); // six=482915
            badCheckDigit = false;
            return true;
        }
    }
    return false;
}
// calls the above functions
bool Decoder::decodeCropping(const cv::Mat &gray, std::string &six) const
{
    badCheckDigit = false;
    six.clear();
    if (gray.empty() || gray.type() != CV_8UC1)
        return false;
    bool badCheck = false;
    // Thin bands tolerate tilted bars, avoids text/scratches crossing one row.
    for (int percent : {50, 40, 60, 30, 70, 20, 80})
    {
        const int y = gray.rows * percent / 100;
        const int top = std::max(0, y - 3);
        const int bottom = std::min(gray.rows, y + 4);
        cv::Mat band;
        cv::copyMakeBorder(gray.rowRange(top, bottom), band, 3, 3, 12, 12,
                           cv::BORDER_CONSTANT, cv::Scalar(255));
        const auto runs = scanLineRuns(band);
        // A barcode can be surrounded by other dark objects on the scan line.
        for (size_t start = 0; start + stripes <= runs.size(); start += 2)
        {
            const vector<double> candidate(runs.begin() + start, runs.begin() + start + stripes);
            const string bits = runsToModules(candidate);
            if (!bits.empty() && decodeSix(bits, six))
                return true;
            badCheck = badCheck || badCheckDigit;
        }
    }
    badCheckDigit = badCheck;
    return false;
}
// displays 'badCheck' for a bad check digit
// else shows error in terminal
bool Decoder::isBadCheck() const
{
    return badCheckDigit;
}
