#include "fungsi.h"


Point::Point() : x(0) , y(0) {}
Point::Point(int x,int y) : x(x), y(y) {}

bool operator==(const Point& lhs, const Point& rhs)
{
    return (lhs.x == rhs.x) && (lhs.y == rhs.y);
}

Point operator+(const Point& lhs, const Point& rhs)
{
    return Point(lhs.x + rhs.x , lhs.y + rhs.y);
}


jobjectArray createJavaArray(JNIEnv *env, jsize count, std::string elements[]){
    jclass stringClass = env->FindClass("java/lang/String");
    jobjectArray row = env->NewObjectArray(count, stringClass, env->NewStringUTF(""));
    jsize i;

    for (i = 0; i < count; i++) {
        env->SetObjectArrayElement( row, i, env->NewStringUTF(elements[i].c_str()));
    }
    return row;
}

uint32_t* createHistogram(NativeBitmap* nBitmap){
	if (nBitmap->pixels == NULL)
    	return NULL;

	uint32_t* result = new uint32_t[256];

	for (int i=0; i<256; i++)
		result[i] = 0;

	uint32_t nBitmapSize = nBitmap->bitmapInfo.height * nBitmap->bitmapInfo.width;

	for (uint32_t i = 0; i < nBitmapSize; i++){
		ARGB bitmapColor;
		convertIntToArgb(nBitmap->pixels[i], &bitmapColor);
		result[bitmapColor.red]++;
	}

	return result;
}