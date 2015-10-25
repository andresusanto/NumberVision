#include "nativebitmap.h"

NativeBitmap::NativeBitmap(){
	pixels = NULL;
}

NativeBitmap::NativeBitmap(NativeBitmap* nBitmap){
	this->bitmapInfo = nBitmap->bitmapInfo;
	uint32_t pixelsCount = this->bitmapInfo.height * this->bitmapInfo.width;

	this->pixels = new uint32_t[pixelsCount];
	memcpy(this->pixels, nBitmap->pixels, sizeof(uint32_t) * pixelsCount);
}


uint32_t convertArgbToInt(ARGB argb) {
    return (argb.alpha << 24) | (argb.red) | (argb.green << 8) | (argb.blue << 16);
}

void convertIntToArgb(uint32_t pixel, ARGB* argb){
    argb->red = ((pixel) & 0xff);
    argb->green = ((pixel >> 8) & 0xff);
    argb->blue = ((pixel >> 16) & 0xff);
    argb->alpha = ((pixel >> 24) & 0xff);
}


NativeBitmap* grayscaleBitmap(NativeBitmap* source){
	NativeBitmap* result = new NativeBitmap(source);
	uint32_t nBitmapSize = source->bitmapInfo.height * source->bitmapInfo.width;


	for (uint32_t i = 0; i < nBitmapSize; i++){
		ARGB bitmapColor;
		convertIntToArgb(source->pixels[i], &bitmapColor);

		uint8_t grayscaleColor = (int)(0.2989f * bitmapColor.red + 0.5870f * bitmapColor.green + 0.1141 * bitmapColor.blue);

		bitmapColor.red = grayscaleColor;
		bitmapColor.green = grayscaleColor;
		bitmapColor.blue = grayscaleColor;

		result->pixels[i] = convertArgbToInt(bitmapColor);
	}

	return result;
}

NativeBitmap* convertBitmapToNative(JNIEnv * env, jobject bitmap){
    AndroidBitmapInfo bitmapInfo;
    uint32_t* storedBitmapPixels = NULL;

    int ret;
    if ((ret = AndroidBitmap_getInfo(env, bitmap, &bitmapInfo)) < 0){
        LOGE("Error eksekusi AndroidBitmap_getInfo()! error=%d", ret);
        return NULL;
    }

    LOGD("width:%d height:%d stride:%d", bitmapInfo.width, bitmapInfo.height, bitmapInfo.stride);

    if (bitmapInfo.format != ANDROID_BITMAP_FORMAT_RGBA_8888){
        LOGE("Format bitmap bukan RGBA_8888!");
        return NULL;
    }

    void* bitmapPixels;
    if ((ret = AndroidBitmap_lockPixels(env, bitmap, &bitmapPixels)) < 0){
        LOGE("Error eksekusi AndroidBitmap_lockPixels()! error=%d", ret);
        return NULL;
    }

    uint32_t* src = (uint32_t*) bitmapPixels;
    storedBitmapPixels = new uint32_t[bitmapInfo.height * bitmapInfo.width];
    uint32_t pixelsCount = bitmapInfo.height * bitmapInfo.width;
    memcpy(storedBitmapPixels, src, sizeof(uint32_t) * pixelsCount);
    AndroidBitmap_unlockPixels(env, bitmap);

    // store ke memory sbg array int
    NativeBitmap *nBitmap = new NativeBitmap();
    nBitmap->bitmapInfo = bitmapInfo;
    nBitmap->pixels = storedBitmapPixels;
    return nBitmap;
}

jobject convertNativeToBitmap(JNIEnv * env, NativeBitmap* nBitmap){
    if (nBitmap->pixels == NULL){
        LOGD("Bitmap kosong / error");
        return NULL;
    }

    // manggil fungsi bitmap java via env
    jclass bitmapCls = env->FindClass("android/graphics/Bitmap");
    jmethodID createBitmapFunction = env->GetStaticMethodID(bitmapCls, "createBitmap", "(IILandroid/graphics/Bitmap$Config;)Landroid/graphics/Bitmap;");
    jstring configName = env->NewStringUTF("ARGB_8888");
    jclass bitmapConfigClass = env->FindClass("android/graphics/Bitmap$Config");
    jmethodID valueOfBitmapConfigFunction = env->GetStaticMethodID(bitmapConfigClass, "valueOf","(Ljava/lang/String;)Landroid/graphics/Bitmap$Config;");
    jobject bitmapConfig = env->CallStaticObjectMethod(bitmapConfigClass, valueOfBitmapConfigFunction, configName);
    jobject newBitmap = env->CallStaticObjectMethod(bitmapCls, createBitmapFunction, nBitmap->bitmapInfo.width, nBitmap->bitmapInfo.height, bitmapConfig);

    // masukin pixel ke bitmap
    int ret;
    void* bitmapPixels;

    if ((ret = AndroidBitmap_lockPixels(env, newBitmap, &bitmapPixels)) < 0){
        LOGE("AndroidBitmap_lockPixels() failed ! error=%d", ret);
        return NULL;
    }

    uint32_t* newBitmapPixels = (uint32_t*) bitmapPixels;
    uint32_t pixelsCount = nBitmap->bitmapInfo.height * nBitmap->bitmapInfo.width;
    memcpy(newBitmapPixels, nBitmap->pixels, sizeof(uint32_t) * pixelsCount);
    AndroidBitmap_unlockPixels(env, newBitmap);

    //LOGD("convert berhasil");
    return newBitmap;

}

NativeBitmap* transformNativeBitmap(NativeBitmap* source, uint32_t* transform){
	NativeBitmap* result = new NativeBitmap(source);
	uint32_t nBitmapSize = source->bitmapInfo.height * source->bitmapInfo.width;


	for (uint32_t i = 0; i < nBitmapSize; i++){
		ARGB bitmapColor;
		convertIntToArgb(source->pixels[i], &bitmapColor);

		bitmapColor.red = transform[bitmapColor.red];
		bitmapColor.green = transform[bitmapColor.green];
		bitmapColor.blue = transform[bitmapColor.blue];

		result->pixels[i] = convertArgbToInt(bitmapColor);
	}
	
	delete[] transform;
	return result;
}
