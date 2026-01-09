cmake_minimum_required(VERSION 3.22.1)
project("stereo_camera")

# 1. SET OPENCV PATH (Update this to your actual path)
# The path must end in /sdk/native/jni
set(OpenCV_DIR "/home/abhinand/Downloads/OpenCV-android-sdk/sdk/native/jni")

# 2. FIND OPENCV
find_package(OpenCV REQUIRED)

# 3. DEFINE THE SHARED LIBRARY
# We use the name 'stereo_camera' so it matches System.loadLibrary("stereo_camera")
add_library(stereo_camera SHARED
        stereo_camera.cpp
        camera_manager.cpp)

# 4. INCLUDE HEADERS
# This tells the compiler where to find <opencv2/opencv.hpp> and "camera_manager.h"
target_include_directories(stereo_camera PRIVATE
        ${OpenCV_INCLUDE_DIRS}
        ${CMAKE_CURRENT_SOURCE_DIR})

# In app/src/main/cpp/CMakeLists.txt

target_link_libraries(stereo_camera
        ${OpenCV_LIBS}
        android
        log
        jnigraphics
        camera2ndk  # <--- Changed from 'camera' to 'camera2ndk'
        mediandk    # <--- Correct for ImageReader
)
