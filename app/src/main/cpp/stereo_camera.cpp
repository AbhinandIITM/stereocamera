#include <jni.h>
#include <android/bitmap.h>
#include <opencv2/opencv.hpp>
#include "camera_manager.h"

static CameraManager g_cameraManager;

extern "C" JNIEXPORT jboolean JNICALL
Java_com_example_stereocamera_MainActivity_initializeCameras(JNIEnv* env, jobject, jstring idL, jstring idR) {
    const char* cid = env->GetStringUTFChars(idL, nullptr);
    bool res = g_cameraManager.initialize(cid);
    env->ReleaseStringUTFChars(idL, cid);
    return res;
}

extern "C" JNIEXPORT void JNICALL
Java_com_example_stereocamera_MainActivity_getSingleFrame(JNIEnv* env, jobject, jobject bitmap) {
    cv::Mat frame = g_cameraManager.getFrame();
    if (frame.empty()) return;

    AndroidBitmapInfo info;
    void* pixels;
    if (AndroidBitmap_getInfo(env, bitmap, &info) < 0) return;
    if (AndroidBitmap_lockPixels(env, bitmap, &pixels) < 0) return;

    // The bitmap is RGBA, and our frame is now RGBA
    cv::Mat bmpMat(info.height, info.width, CV_8UC4, pixels);
    cv::Mat resized;
    cv::resize(frame, resized, cv::Size(info.width, info.height));

    // Copy the colored frame directly
    resized.copyTo(bmpMat);

    AndroidBitmap_unlockPixels(env, bitmap);
}


extern "C" JNIEXPORT void JNICALL
Java_com_example_stereocamera_MainActivity_releaseCameras(JNIEnv* env, jobject) {
    g_cameraManager.stopCapture();
}
