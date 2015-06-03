#define LOG_TAG "a22301"

#include <stdio.h>
//#include <cutils/log.h>
#include <android/log.h>
#  define LOGE(...) ((void)__android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__))

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

#include "com_caredear_myocr_OcrUtil.h"

static void foo()
{
}

jstring JNICALL Java_com_caredear_myocr_OcrUtil_getImg (JNIEnv *env, jobject obj, jstring fn)
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

