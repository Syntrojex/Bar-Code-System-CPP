#include "scanner/CameraScanner.h"
#ifdef _WIN32
#include <windows.h>
#include <dshow.h>
#endif

#ifdef HAVE_OPENCV
#include <QElapsedTimer>
#include <opencv2/imgproc.hpp>
#include "Scanner.h"
#endif

CameraScanner::CameraScanner()
{
#ifdef HAVE_OPENCV
    m_decoder = std::make_unique<Scanner>();        // used only for frameRead(); it never opens its own camera here
#endif
}

CameraScanner::~CameraScanner() = default;

QStringList CameraScanner::availableCameras()
{
    QStringList names;
#ifdef _WIN32
    const HRESULT initialized = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    if (FAILED(initialized) && initialized != RPC_E_CHANGED_MODE) return names;
    ICreateDevEnum *devices = nullptr;
    IEnumMoniker *enumerator = nullptr;
    if (SUCCEEDED(CoCreateInstance(CLSID_SystemDeviceEnum, nullptr, CLSCTX_INPROC_SERVER,
                                  IID_ICreateDevEnum, reinterpret_cast<void **>(&devices)))) {
        if (devices->CreateClassEnumerator(CLSID_VideoInputDeviceCategory, &enumerator, 0) == S_OK) {
            IMoniker *moniker = nullptr;
            while (enumerator->Next(1, &moniker, nullptr) == S_OK) {
                QString name = QString("Camera %1").arg(names.size());
                IPropertyBag *properties = nullptr;
                if (SUCCEEDED(moniker->BindToStorage(nullptr, nullptr, IID_IPropertyBag,
                                                    reinterpret_cast<void **>(&properties)))) {
                    VARIANT value;
                    VariantInit(&value);
                    if (SUCCEEDED(properties->Read(L"FriendlyName", &value, nullptr)) && value.vt == VT_BSTR)
                        name = QString::fromWCharArray(value.bstrVal);
                    VariantClear(&value);
                    properties->Release();
                }
                names << name;
                moniker->Release();
            }
            enumerator->Release();
        }
        devices->Release();
    }
    if (SUCCEEDED(initialized)) CoUninitialize();
#else
    // Backend indices on platforms without DirectShow enumeration.
    names << "Camera 0" << "Camera 1" << "Camera 2";
#endif
    return names;
}

bool CameraScanner::openCamera(int cameraIndex, QString *error, QSize resolution)
{
#ifdef HAVE_OPENCV
    closeCamera();
    if (error) error->clear();
    if (cameraIndex < 0) {
        if (error) *error = "No camera selected.";
        return false;
    }
    try {
#ifdef _WIN32
    const bool opened = capture.open(cameraIndex, cv::CAP_DSHOW);   // DirectShow: fast start, same backend as Backend/Scanner
#else
    const bool opened = capture.open(cameraIndex);
#endif
    if (opened) {
        capture.set(cv::CAP_PROP_FRAME_WIDTH, resolution.width());
        capture.set(cv::CAP_PROP_FRAME_HEIGHT, resolution.height());
        capture.set(cv::CAP_PROP_FPS, 30);
        autofocus();
        return true;
    }
    } catch (const cv::Exception &e) {
        if (error) *error = QString::fromStdString(e.what());
        closeCamera();
        return false;
    }
    if (error) *error = QString("Failed to open camera %1. Check that it is connected and not used by another app.").arg(cameraIndex);
    return false;
#else
    Q_UNUSED(cameraIndex);
    Q_UNUSED(resolution);
    if (error) *error = "Camera support requires OpenCV.";
    return false;
#endif
}

CameraFrameResult CameraScanner::readFrame(bool decode)
{
    CameraFrameResult res;
#ifdef HAVE_OPENCV
    try {
    if (!capture.isOpened()) {
        res.error = "Camera not opened";
        return res;
    }
    cv::Mat mat;
    if (capture.read(mat) && !mat.empty()) {
        res.success = true;

        QElapsedTimer timer;
        timer.start();
        std::string six;
        cv::Rect box;
        if (decode && m_decoder->frameRead(six, box, mat))
            res.decodedText = QString::fromStdString(six);
        if (box.area() > 0)
            res.box = QRect(box.x, box.y, box.width, box.height);
        res.decodeMs = static_cast<int>(timer.elapsed());

        cv::Mat rgb;
        cv::cvtColor(mat, rgb, cv::COLOR_BGR2RGB);
        res.frame = QImage(rgb.data, rgb.cols, rgb.rows, static_cast<int>(rgb.step), QImage::Format_RGB888).copy();
    } else {
        res.error = "Camera stopped delivering frames.";
    }
    } catch (const cv::Exception &e) {
        res.success = false;
        res.error = QString::fromStdString(e.what());
    }
#else
    res.success = m_opened;
#endif
    return res;
}

bool CameraScanner::autofocus()
{
#ifdef HAVE_OPENCV
    try { return capture.isOpened() && capture.set(cv::CAP_PROP_AUTOFOCUS, 1); }
    catch (const cv::Exception &) { return false; }
#else
    return false;
#endif
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
