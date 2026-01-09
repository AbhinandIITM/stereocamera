#ifndef CAMERA_MANAGER_H
#define CAMERA_MANAGER_H

#include <camera/NdkCameraManager.h>
#include <camera/NdkCameraDevice.h>
#include <camera/NdkCameraCaptureSession.h>
#include <media/NdkImageReader.h>
#include <opencv2/opencv.hpp>
#include <mutex>

class CameraManager {
public:
    CameraManager();
    ~CameraManager();

    bool initialize(const char* cameraId);
    void stopCapture();
    cv::Mat getFrame();

private:
    bool openCamera(const char* id, ACameraDevice** device);
    bool createSession(ACameraDevice* device, AImageReader* reader, ACameraCaptureSession** session);
    static void onImageAvailable(void* context, AImageReader* reader);

    ACameraManager* m_manager;
    ACameraDevice* m_device;
    AImageReader* m_reader;
    ACameraCaptureSession* m_session;

    std::mutex m_mtx;
    cv::Mat m_currentFrame;
};

#endif
