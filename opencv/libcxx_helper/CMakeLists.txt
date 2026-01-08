cmake_minimum_required(VERSION 3.10)
project(opencv_jni_shared) # This is the missing command

# dummy target to bring libc++_shared.so into packages
add_library(opencv_jni_shared STATIC dummy.cpp)
