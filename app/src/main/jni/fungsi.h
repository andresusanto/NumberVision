// EQUALIGRAM - EQUALIZER INSTAGRAM
// Andre Susanto, M Yafi, Ramandika P, Kevin Yudi
// Pengcit - IF ITB

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
#include <sstream>
#include <fstream>

#include "nativebitmap.h"

#define  LOG_TAG    "NUMBER VISION"
#define  LOGD(...)  __android_log_print(ANDROID_LOG_DEBUG,LOG_TAG,__VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)

using namespace std;

typedef struct{
    char value;
    string chain; // untuk keperluan learning... mager :D
    int start;
} DetectedChar;

typedef struct{
    char direction;
    int value;
} ECode;

typedef struct{
    char meaning;
    string data;
} Knowledge;

class Point{
    public:
	int x;
    int y;

    Point();
    Point(int x,int y);
};

typedef struct border_info{
    Point start_point;
    vector<int> chain_codes;
} BorderInfo;

bool operator==(const Point& lhs, const Point& rhs);
Point operator+(const Point& lhs, const Point& rhs);


jobjectArray createJavaArray(JNIEnv *env, jsize count, std::string elements[]);
uint32_t* createHistogram(NativeBitmap* nBitmap);