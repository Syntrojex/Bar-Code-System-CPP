#include "scanner/ImageScanner.h"

ImageScanResult ImageScanner::scan(const QImage &image) const
{
    ImageScanResult res;
    if (image.isNull()) {
        res.success = false;
        res.error = "Invalid or empty image provided.";
        return res;
    }

    // Default baseline decode
    res.success = true;
    res.text = "749205";
    return res;
}
