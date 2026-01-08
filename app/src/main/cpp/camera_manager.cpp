#include "camera_manager.h"
#include <android/log.h>

#define LOG_TAG "StereoCamera"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

CameraManager::CameraManager() : m_manager(nullptr), m_deviceLeft(nullptr), m_deviceRight(nullptr),
                                 m_readerLeft(nullptr), m_readerRight(nullptr),
                                 m_sessionLeft(nullptr), m_sessionRight(nullptr) {
    m_manager = ACameraManager_create();
}

CameraManager::~CameraManager() {
    stopCapture();
    if (m_manager) ACameraManager_delete(m_manager);
}

bool CameraManager::initialize(const char* idLeft, const char* idRight) {
    // Attempt to open the primary camera (idLeft)
    if (!openCamera(idLeft, &m_deviceLeft)) return false;

    // Use a safer resolution for testing
    AImageReader_new(320, 240, AIMAGE_FORMAT_YUV_420_888, 2, &m_readerLeft);

    AImageReader_ImageCallback cb = { .context = this, .onImageAvailable = onImageAvailable };
    AImageReader_setImageListener(m_readerLeft, &cb);

    bool leftOk = createSession(m_deviceLeft, m_readerLeft, &m_sessionLeft);

    // Attempt to open the secondary camera, but don't crash if it fails
    if (idRight != nullptr && std::string(idLeft) != std::string(idRight)) {
        if (openCamera(idRight, &m_deviceRight)) {
            AImageReader_new(320, 240, AIMAGE_FORMAT_YUV_420_888, 2, &m_readerRight);
            AImageReader_setImageListener(m_readerRight, &cb);
            createSession(m_deviceRight, m_readerRight, &m_sessionRight);
        }
    }

    return leftOk;
}

bool CameraManager::openCamera(const char* id, ACameraDevice** device) {
    ACameraDevice_StateCallbacks cb = { .context = this, .onDisconnected = nullptr, .onError = nullptr };
    return ACameraManager_openCamera(m_manager, id, &cb, device) == ACAMERA_OK;
}

bool CameraManager::createSession(ACameraDevice* device, AImageReader* reader, ACameraCaptureSession** session) {
    if (device == nullptr || reader == nullptr) return false;

    ANativeWindow* window;
    AImageReader_getWindow(reader, &window);
    ACameraOutputTarget* target;
    ACameraOutputTarget_create(window, &target);

    ACaptureRequest* request;
    ACameraDevice_createCaptureRequest(device, TEMPLATE_PREVIEW, &request);
    ACaptureRequest_addTarget(request, target);

    ACameraDevice_createCaptureSession(device, &target, 1, nullptr, session);
    ACameraCaptureSession_setRepeatingRequest(*session, nullptr, 1, &request, nullptr);
    return true;
}
// ... onImageAvailable and getter logic remains similar but with null checks
