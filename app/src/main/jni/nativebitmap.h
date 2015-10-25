#pragma once

#include <jni.h>
#include <android/log.h>
#include <stdio.h>
#include <android/bitmap.h>
#include <string>
#include <unistd.h>
#include <cmath>
#include <stdlib.h>
#include <algorithm>
#include <math.h>
#include <queue>
#include <sstream>

#define  LOG_TAG    "NUMBER VISION"
#define  LOGD(...)  __android_log_print(ANDROID_LOG_DEBUG,LOG_TAG,__VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)

using namespace std;

class NativeBitmap{
public:
    uint32_t* pixels;
    AndroidBitmapInfo bitmapInfo;

    NativeBitmap();
    NativeBitmap(NativeBitmap* nBitmap);
};


typedef struct{
	uint8_t alpha, red, green, blue;
} ARGB;


uint32_t convertArgbToInt(ARGB argb);
void convertIntToArgb(uint32_t pixel, ARGB* argb);


NativeBitmap* grayscaleBitmap(NativeBitmap* source);
NativeBitmap* convertBitmapToNative(JNIEnv * env, jobject bitmap);
jobject convertNativeToBitmap(JNIEnv * env, NativeBitmap* nBitmap);
NativeBitmap* transformNativeBitmap(NativeBitmap* source, uint32_t* transform);
