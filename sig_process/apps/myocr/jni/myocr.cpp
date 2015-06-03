#define LOG_TAG "a22301"

#include <stdio.h>
//#include <cutils/log.h>
#include <android/log.h>
#  define LOGE(...) ((void)__android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__))

//TODO in final version, would mask the LOGD
#define LOGD LOGE

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

#include "com_caredear_myocr_OcrUtil.h"

/*
 * Class:     com_caredear_myocr_OcrUtil
 * Method:    processTargetImg
 * Signature: (Ljava/lang/String;IIII)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_com_caredear_myocr_OcrUtil_processTargetImg
  (JNIEnv *env, jclass cls, jstring imgName, jint x1, jint y1, jint x2, jint y2)
{
    jstring ret;
    const char *filename = env->GetStringUTFChars(imgName, NULL);
    LOGE("Java --->C++:%s", filename);

    cv::Mat img = cv::imread(filename, CV_LOAD_IMAGE_GRAYSCALE);
    if(!img.data)
    {
        LOGE("**failed open %s image", filename);
        ret = env->NewStringUTF("image failed read in cv");
        return ret;
    }
    cv::Range r1(x1, y1);
    cv::Range r2(x2, y2);
    cv::Mat cropped = img(r1, r2);
    // keep processing the cropped image...

    ret = env->NewStringUTF("FOO");

    return ret;
}

/**
 * return a binary balck-white image file name, so OCR
 * can recognize it later at Java side.
 */
JNIEXPORT jstring JNICALL Java_com_caredear_myocr_OcrUtil_matchingTextOnImg (JNIEnv *env, jclass cls, jstring fn)
{
    jstring ret;
    const char *filename = env->GetStringUTFChars(fn, NULL);
    LOGE("Java --->C++:%s", filename);
    ret = env->NewStringUTF("FOO");

    return ret;
}

jstring JNICALL Java_com_caredear_myocr_OcrUtil_getImg (JNIEnv *env, jclass obj, jstring fn)
{
    jstring ret;
    const char *filename = env->GetStringUTFChars(fn, NULL);
    LOGE("Java --->C++:%s", filename);
    cv::Mat img = cv::imread(filename, CV_LOAD_IMAGE_GRAYSCALE);
    if(!img.data)
    {
        LOGE("***failed open the %s image\n", filename);
        ret = env->NewStringUTF("open file failure");
    }
    else
    {
        LOGE("open file [OK]\n");
        LOGE("width:%d, height:%d", img.cols, img.rows);
        bool rc = cv::imwrite("/mnt/sdcard/cv.png", img);
        if(rc)
            ret = env->NewStringUTF("write file OK");
        else
            ret = env->NewStringUTF("false for write");
    }


    return ret;
}

