#include <jni.h>
#include <android/bitmap.h>
#include <opencv2/opencv.hpp>
#include "camera_manager.h"

static CameraManager g_cameraManager;

extern "C" JNIEXPORT jboolean JNICALL
Java_com_example_stereocamera_MainActivity_initializeCameras(JNIEnv* env, jobject, jstring idL, jstring idR) {
    const char* left = env->GetStringUTFChars(idL, nullptr);
    const char* right = env->GetStringUTFChars(idR, nullptr);
    bool res = g_cameraManager.initialize(left, right);
    env->ReleaseStringUTFChars(idL, left);
    env->ReleaseStringUTFChars(idR, right);
    return res;
}

extern "C" JNIEXPORT void JNICALL
Java_com_example_stereocamera_MainActivity_getSingleFrame(JNIEnv* env, jobject, jobject bitmap) {
    cv::Mat frame = g_cameraManager.getLeftFrame();
    if (frame.empty()) return;

    AndroidBitmapInfo info;
    void* pixels;
    AndroidBitmap_getInfo(env, bitmap, &info);
    AndroidBitmap_lockPixels(env, bitmap, &pixels);

    cv::Mat bmpMat(info.height, info.width, CV_8UC4, pixels);
    // Convert grayscale Y-plane to RGBA for display
    cv::cvtColor(frame, bmpMat, cv::COLOR_GRAY2RGBA);

    AndroidBitmap_unlockPixels(env, bitmap);
}

extern "C" JNIEXPORT void JNICALL
Java_com_example_stereocamera_MainActivity_releaseCameras(JNIEnv* env, jobject) {
    g_cameraManager.stopCapture();
}
