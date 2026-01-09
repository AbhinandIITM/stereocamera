#include "camera_manager.h"
#include <android/log.h>

#define LOG_TAG "StereoCamera"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

CameraManager::CameraManager() : m_manager(nullptr), m_device(nullptr), m_reader(nullptr), m_session(nullptr) {
    m_manager = ACameraManager_create();
}

CameraManager::~CameraManager() {
    stopCapture();
    if (m_manager) ACameraManager_delete(m_manager);
}

bool CameraManager::initialize(const char* cameraId) {
    if (!openCamera(cameraId, &m_device)) return false;

    AImageReader_new(640, 480, AIMAGE_FORMAT_YUV_420_888, 2, &m_reader);

    AImageReader_ImageListener listener = {};
    listener.context = this;
    listener.onImageAvailable = onImageAvailable;
    AImageReader_setImageListener(m_reader, &listener);

    return createSession(m_device, m_reader, &m_session);
}
bool CameraManager::openCamera(const char* id, ACameraDevice** device) {
    ACameraDevice_StateCallbacks cb = {}; // Initialize to zero
    cb.context = this;
    // Provide empty handlers instead of raw nulls if necessary
    cb.onDisconnected = [](void* context, ACameraDevice* device) {
        LOGE("Camera Disconnected");
    };
    cb.onError = [](void* context, ACameraDevice* device, int error) {
        LOGE("Camera Error: %d", error);
    };

    return ACameraManager_openCamera(m_manager, id, &cb, device) == ACAMERA_OK;
}

bool CameraManager::createSession(ACameraDevice* device, AImageReader* reader, ACameraCaptureSession** session) {
    if (!device || !reader) return false;

    ANativeWindow* window = nullptr;
    media_status_t windowStatus = AImageReader_getWindow(reader, &window);
    if (windowStatus != AMEDIA_OK || window == nullptr) {
        LOGE("Failed to get ANativeWindow from ImageReader");
        return false;
    }

    // 1. Create session output container
    ACaptureSessionOutputContainer* container = nullptr;
    ACaptureSessionOutputContainer_create(&container);

    ACaptureSessionOutput* output = nullptr;
    ACaptureSessionOutput_create(window, &output);
    ACaptureSessionOutputContainer_add(container, output);

    // 2. Create the target for capture request
    ACameraOutputTarget* target = nullptr;
    ACameraOutputTarget_create(window, &target);

    ACaptureRequest* request = nullptr;
    // Template changed to TEMPLATE_STILL_CAPTURE for better stability in tests
    ACameraDevice_createCaptureRequest(device, TEMPLATE_PREVIEW, &request);
    ACaptureRequest_addTarget(request, target);

    // 3. Create session with 4 arguments
    // Important: Pass empty callbacks struct instead of nullptr for some devices
    ACameraCaptureSession_stateCallbacks sessionCallbacks = {};
    camera_status_t status = ACameraDevice_createCaptureSession(device, container, &sessionCallbacks, session);

    if (status == ACAMERA_OK) {
        ACameraCaptureSession_setRepeatingRequest(*session, nullptr, 1, &request, nullptr);
    } else {
        LOGE("ACameraDevice_createCaptureSession failed with error: %d", status);
    }

    // Cleanup local memory
    ACaptureRequest_free(request);
    ACameraOutputTarget_free(target);
    ACaptureSessionOutput_free(output);
    ACaptureSessionOutputContainer_free(container);

    return status == ACAMERA_OK;
}
void CameraManager::onImageAvailable(void* context, AImageReader* reader) {
    auto* self = static_cast<CameraManager*>(context);
    AImage* image = nullptr;
    if (AImageReader_acquireLatestImage(reader, &image) == AMEDIA_OK) {
        int32_t width, height;
        AImage_getWidth(image, &width);
        AImage_getHeight(image, &height);

        // Plane 0: Y (Luminance)
        uint8_t *yData; int yLen;
        AImage_getPlaneData(image, 0, &yData, &yLen);

        // Plane 1: U (Chrominance)
        uint8_t *uData; int uLen;
        AImage_getPlaneData(image, 1, &uData, &uLen);

        // To get color efficiently, we create a YUV Mat
        // The height is 1.5x original because Y is full size, U/V are half size
        cv::Mat yuvMat(height + height / 2, width, CV_8UC1);

        // Copy Y data
        memcpy(yuvMat.data, yData, width * height);

        // Copy interleaved UV data (NV21 style)
        // Note: For simplicity, we assume NV21/NV12 which is common in NDK
        memcpy(yuvMat.data + (width * height), uData, width * height / 2);

        cv::Mat colorMat;
        // Convert from YUV (NV21) to RGBA
        cv::cvtColor(yuvMat, colorMat, cv::COLOR_YUV2RGBA_NV21);

        // Rotate 90 degrees clockwise
        cv::Mat rotated;
        cv::rotate(colorMat, rotated, cv::ROTATE_90_CLOCKWISE);

        std::lock_guard<std::mutex> lock(self->m_mtx);
        self->m_currentFrame = rotated.clone();

        AImage_delete(image);
    }
}


void CameraManager::stopCapture() {
    if (m_session) ACameraCaptureSession_close(m_session);
    if (m_device) ACameraDevice_close(m_device);
    m_session = nullptr;
    m_device = nullptr;
}

cv::Mat CameraManager::getFrame() {
    std::lock_guard<std::mutex> lock(m_mtx);
    return m_currentFrame.clone();
}
