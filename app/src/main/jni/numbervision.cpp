#include "fungsi.h"
#include "ekualisasi.h"
#include "vision.h"


extern "C"
{
	// ekualigram
	JNIEXPORT jobject JNICALL Java_com_ganesus_numbervision_EqualigramMain_loadBitmap (JNIEnv * env, jobject obj, jobject bitmap);
	JNIEXPORT jobject JNICALL Java_com_ganesus_numbervision_EqualigramMain_genHistogram (JNIEnv * env, jobject obj, jobject bitmem, jobject canvas, bool isGrayScale);
	JNIEXPORT jobject JNICALL Java_com_ganesus_numbervision_EqualigramMain_createGrayscale (JNIEnv * env, jobject obj, jobject bitmem);
	JNIEXPORT jobject JNICALL Java_com_ganesus_numbervision_EqualigramMain_createGrayscaleBmp (JNIEnv * env, jobject obj, jobject bitmem);
	JNIEXPORT jobject JNICALL Java_com_ganesus_numbervision_EqualigramMain_applyAlgo1 (JNIEnv * env, jobject obj, jobject bitmem);
	JNIEXPORT jobject JNICALL Java_com_ganesus_numbervision_EqualigramMain_applyAlgo1Bmp (JNIEnv * env, jobject obj, jobject bitmem);
	JNIEXPORT jobject JNICALL Java_com_ganesus_numbervision_EqualigramMain_applyAlgo2 (JNIEnv * env, jobject obj, jobject bitmem);
	JNIEXPORT jobject JNICALL Java_com_ganesus_numbervision_EqualigramMain_applyAlgo2Bmp (JNIEnv * env, jobject obj, jobject bitmem);
	JNIEXPORT jobject JNICALL Java_com_ganesus_numbervision_EqualigramMain_applyAlgoLinear (JNIEnv * env, jobject obj, jobject bitmem,int new_min,int new_max);
	JNIEXPORT jobject JNICALL Java_com_ganesus_numbervision_EqualigramMain_applyAlgoLinearBmp (JNIEnv * env, jobject obj, jobject bitmem,int new_min,int new_max);
	JNIEXPORT jobject JNICALL Java_com_ganesus_numbervision_EqualigramMain_applyAlgoStep (JNIEnv * env, jobject obj, jobject bitmem, int L);
	JNIEXPORT jobject JNICALL Java_com_ganesus_numbervision_EqualigramMain_applyAlgoStepBmp (JNIEnv * env, jobject obj, jobject bitmem, int L);

	// vision 1
	JNIEXPORT jobjectArray JNICALL Java_com_ganesus_numbervision_Vision1_detectAll (JNIEnv * env, jobject obj, jobject bitmap);

	// vision 2
	JNIEXPORT jobjectArray JNICALL Java_com_ganesus_numbervision_Vision2_detectAll (JNIEnv * env, jobject obj, jobject bitmap, jstring path);
    JNIEXPORT jobject JNICALL Java_com_ganesus_numbervision_Vision2_preProses (JNIEnv * env, jobject obj, jobject bitmap, jobject canvas);

}


/////////////////////////////////////////////// BAGIAN EQUALIGRAM ////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////////////
// fungsi untuk load bitmap dan store ke native memory
/////////////////////////////////////////////////////////////////////////////////////

JNIEXPORT jobject JNICALL Java_com_ganesus_numbervision_EqualigramMain_loadBitmap (JNIEnv * env, jobject obj, jobject bitmap){
    return env->NewDirectByteBuffer(convertBitmapToNative (env, bitmap), 0);
}

/////////////////////////////////////////////////////////////////////////////////////
// fungsi untuk generate histogram 3 warna
/////////////////////////////////////////////////////////////////////////////////////

JNIEXPORT jobject JNICALL Java_com_ganesus_numbervision_EqualigramMain_genHistogram (JNIEnv * env, jobject obj, jobject bitmem, jobject canvas, bool isGrayScale){
	NativeBitmap* nCanvas = convertBitmapToNative(env, canvas);
	NativeBitmap* nBitmap = (NativeBitmap*) env->GetDirectBufferAddress(bitmem);


	if (nBitmap->pixels == NULL || nCanvas->pixels == NULL)
		return NULL;

	uint32_t* hRed = new uint32_t[256];
	uint32_t* hGreen = new uint32_t[256];
	uint32_t* hBlue = new uint32_t[256];

	for (uint16_t i = 0; i < 256; i++){
		hRed[i] = 0; hGreen[i] = 0; hBlue[i] = 0;
	}


	uint32_t nBitmapSize = nBitmap->bitmapInfo.height * nBitmap->bitmapInfo.width;

	for (uint32_t i = 0; i < nBitmapSize; i++){
		ARGB bitmapColor;
		convertIntToArgb(nBitmap->pixels[i], &bitmapColor);

		hRed[bitmapColor.red]++;
		hGreen[bitmapColor.green]++;
		hBlue[bitmapColor.blue]++;
	}

	float scalingBlue = 0.0f, scalingGreen = 0.0f, scalingRed = 0.0f;
	uint32_t blue_max = hBlue[0], green_max = hGreen[0], red_max = hRed[0];


	for (uint16_t i = 0; i < 256; i++){
		if (hBlue[i] > blue_max) blue_max = hBlue[i];
		if (hGreen[i] > green_max) green_max = hGreen[i];
        if (hRed[i] > red_max) red_max = hRed[i];
	}

	scalingBlue = 255.0f / blue_max; scalingGreen = 255.0f / green_max; scalingRed = 255.0f / red_max;

	ARGB aRed, aGreen, aBlue, aGray;
	aRed.red = 255; aRed.green = 0; aRed.blue = 0; aRed.alpha = 255;
	aGreen.red = 0; aGreen.green = 255; aGreen.blue = 0; aGreen.alpha = 255;
	aBlue.red = 0; aBlue.green = 0; aBlue.blue = 255; aBlue.alpha = 255;
	aGray.red = 127; aGray.green = 127; aGray.blue = 127; aGray.alpha = 255;

	uint32_t iRed= convertArgbToInt(aRed);
	uint32_t iGreen= convertArgbToInt(aGreen);
	uint32_t iBlue= convertArgbToInt(aBlue);
	uint32_t iGray= convertArgbToInt(aGray);

	if (isGrayScale){
		for (uint16_t i = 0; i < 256; i++){
			int barSize = (int) (hRed[i] * scalingRed);
			for (int j = 0; j < barSize; j++){
				nCanvas->pixels[i * 2 + (300 - j) * nCanvas->bitmapInfo.width] = iGray;
				nCanvas->pixels[i * 2 + 1 + (300 - j) * nCanvas->bitmapInfo.width] = iGray;
			}
		}
	}else{
		for (uint16_t i = 0; i < 256; i++){
			int barSize = (int) (hRed[i] * scalingRed);
			for (int j = 0; j < barSize; j++){
				nCanvas->pixels[i + (300 - j) * nCanvas->bitmapInfo.width] = iRed;
			}

			barSize = (int) (hGreen[i] * scalingGreen);
			for (int j = 0; j < barSize; j++){
				nCanvas->pixels[260 + i + (300 - j) * nCanvas->bitmapInfo.width] = iGreen;
			}

			barSize = (int) (hBlue[i] * scalingBlue);
			for (int j = 0; j < barSize; j++){
				nCanvas->pixels[520 + i + (300 - j) * nCanvas->bitmapInfo.width] = iBlue;
			}
		}
	}
	
	delete[] hRed;
	delete[] hBlue;
	delete[] hGreen;
	//return convertNativeToBitmap(env, grayscaleBitmap(nBitmap));
	return convertNativeToBitmap(env, nCanvas);

}

/////////////////////////////////////////////////////////////////////////////////////
// fungsi untuk generate grayscale bitmap
/////////////////////////////////////////////////////////////////////////////////////

JNIEXPORT jobject JNICALL Java_com_ganesus_numbervision_EqualigramMain_createGrayscaleBmp (JNIEnv * env, jobject obj, jobject bitmem){
	NativeBitmap* nBitmap = (NativeBitmap*) env->GetDirectBufferAddress(bitmem);
	return convertNativeToBitmap(env, grayscaleBitmap(nBitmap));
}


/////////////////////////////////////////////////////////////////////////////////////
// fungsi untuk generate grayscale native bitmap
/////////////////////////////////////////////////////////////////////////////////////

JNIEXPORT jobject JNICALL Java_com_ganesus_numbervision_EqualigramMain_createGrayscale (JNIEnv * env, jobject obj, jobject bitmem){
	NativeBitmap* nBitmap = (NativeBitmap*) env->GetDirectBufferAddress(bitmem);
	return env->NewDirectByteBuffer(grayscaleBitmap(nBitmap), 0);
}

/////////////////////////////////////////////////////////////////////////////////////
// fungsi untuk menerapkan algoritma 1 ke native bitmap grayscale
/////////////////////////////////////////////////////////////////////////////////////

JNIEXPORT jobject JNICALL Java_com_ganesus_numbervision_EqualigramMain_applyAlgo1 (JNIEnv * env, jobject obj, jobject bitmem){
	NativeBitmap* nBitmap = (NativeBitmap*) env->GetDirectBufferAddress(bitmem);

	uint32_t* histogram = createHistogram(nBitmap);
	//uint32_t* color_transform = simple_equalization(histogram);
	uint32_t* color_transform = cumulative_equalization(histogram);

	delete[] histogram;
	return env->NewDirectByteBuffer(transformNativeBitmap(nBitmap, color_transform), 0);
}

JNIEXPORT jobject JNICALL Java_com_ganesus_numbervision_EqualigramMain_applyAlgo1Bmp (JNIEnv * env, jobject obj, jobject bitmem){
	NativeBitmap* nBitmap = (NativeBitmap*) env->GetDirectBufferAddress(bitmem);

	uint32_t* histogram = createHistogram(nBitmap);
	//uint32_t* color_transform = simple_equalization(histogram);
	uint32_t* color_transform = cumulative_equalization(histogram);

	delete[] histogram;
	return convertNativeToBitmap(env, transformNativeBitmap(nBitmap, color_transform));
}

/////////////////////////////////////////////////////////////////////////////////////
// fungsi untuk menerapkan algoritma 2 ke native bitmap grayscale
/////////////////////////////////////////////////////////////////////////////////////


JNIEXPORT jobject JNICALL Java_com_ganesus_numbervision_EqualigramMain_applyAlgo2 (JNIEnv * env, jobject obj, jobject bitmem){
	NativeBitmap* nBitmap = (NativeBitmap*) env->GetDirectBufferAddress(bitmem);

	uint32_t* histogram = createHistogram(nBitmap);
	uint32_t* color_transform = simple_equalization(histogram);

	delete[] histogram;
	return env->NewDirectByteBuffer(transformNativeBitmap(nBitmap, color_transform), 0);
}

JNIEXPORT jobject JNICALL Java_com_ganesus_numbervision_EqualigramMain_applyAlgo2Bmp (JNIEnv * env, jobject obj, jobject bitmem){
	NativeBitmap* nBitmap = (NativeBitmap*) env->GetDirectBufferAddress(bitmem);

	uint32_t* histogram = createHistogram(nBitmap);
	uint32_t* color_transform = simple_equalization(histogram);

	delete[] histogram;
	return convertNativeToBitmap(env, transformNativeBitmap(nBitmap, color_transform));
}


/////////////////////////////////////////////////////////////////////////////////////
// fungsi untuk menerapkan algoritma linear ke native bitmap grayscale
/////////////////////////////////////////////////////////////////////////////////////


JNIEXPORT jobject JNICALL Java_com_ganesus_numbervision_EqualigramMain_applyAlgoLinear (JNIEnv * env, jobject obj, jobject bitmem,int new_min,int new_max){
	NativeBitmap* nBitmap = (NativeBitmap*) env->GetDirectBufferAddress(bitmem);

	uint32_t* color_transform = new uint32_t[256];
	uint32_t* histogram = createHistogram(nBitmap);
	int old_min = get_lower_bound(histogram);
	int old_max = get_upper_bound(histogram);

	for(int i = 0; i < 256; i++){
		color_transform[i] = NOT_SPECIFIED_COLOR;
		if (old_min <= i && i <= old_max){
			color_transform[i] = adjustIntensity(i,old_min,old_max,new_min,new_max);
		}
	}
	return env->NewDirectByteBuffer(transformNativeBitmap(nBitmap,color_transform),0);
}

JNIEXPORT jobject JNICALL Java_com_ganesus_numbervision_EqualigramMain_applyAlgoLinearBmp (JNIEnv * env, jobject obj, jobject bitmem,int new_min,int new_max){
	NativeBitmap* nBitmap = (NativeBitmap*) env->GetDirectBufferAddress(bitmem);

	uint32_t* color_transform = new uint32_t[256];
	uint32_t* histogram = createHistogram(nBitmap);
	int old_min = get_lower_bound(histogram);
	int old_max = get_upper_bound(histogram);

	for(int i = 0; i < 256; i++){
		color_transform[i] = NOT_SPECIFIED_COLOR;
		if (old_min <= i && i <= old_max){
			color_transform[i] = adjustIntensity(i,old_min,old_max,new_min,new_max);
		}
	}

	return convertNativeToBitmap(env,transformNativeBitmap(nBitmap,color_transform));
}

/////////////////////////////////////////////////////////////////////////////////////
// fungsi untuk menerapkan algoritma step ke native bitmap grayscale
/////////////////////////////////////////////////////////////////////////////////////


JNIEXPORT jobject JNICALL Java_com_ganesus_numbervision_EqualigramMain_applyAlgoStep (JNIEnv * env, jobject obj, jobject bitmem, int L){
	NativeBitmap* nBitmap = (NativeBitmap*) env->GetDirectBufferAddress(bitmem);

	uint32_t* histogram = createHistogram(nBitmap);
	uint32_t* color_transform = equalize_step(histogram, L);

	delete[] histogram;
	return env->NewDirectByteBuffer(transformNativeBitmap(nBitmap, color_transform), 0);
}

JNIEXPORT jobject JNICALL Java_com_ganesus_numbervision_EqualigramMain_applyAlgoStepBmp (JNIEnv * env, jobject obj, jobject bitmem, int L){
	NativeBitmap* nBitmap = (NativeBitmap*) env->GetDirectBufferAddress(bitmem);

	uint32_t* histogram = createHistogram(nBitmap);
	uint32_t* color_transform = equalize_step(histogram, L);

	delete[] histogram;
	return convertNativeToBitmap(env,transformNativeBitmap(nBitmap,color_transform));
}







////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//		VISION PERTAMA

JNIEXPORT jobjectArray JNICALL Java_com_ganesus_numbervision_Vision1_detectAll (JNIEnv * env, jobject obj, jobject bitmap){
    NativeBitmap* nativeBitmap = convertBitmapToNative (env, bitmap);
    bool **image;

    image = new bool*[nativeBitmap->bitmapInfo.height];
    for (int i=0;i<nativeBitmap->bitmapInfo.height;i++) {
        image[i] = new bool[nativeBitmap->bitmapInfo.width];
        for (int j=0;j<nativeBitmap->bitmapInfo.width;j++) {
            ARGB warna;
            convertIntToArgb(nativeBitmap->pixels[i * nativeBitmap->bitmapInfo.width + j], &warna);

            image[i][j] = !(warna.blue == 255 && warna.green == 255 && warna.red == 255);
        }
    }

    vector<DetectedChar> interpretation;
    vector<BorderInfo> border_infos = get_border_infos(image,nativeBitmap->bitmapInfo.width,nativeBitmap->bitmapInfo.height);
    delete nativeBitmap;


    for (int i=0;i<border_infos.size();i++) {
        BorderInfo border_info = border_infos[i];
        stringstream sskode;

        for (int j=0;j<border_info.chain_codes.size();j++) {
            sskode << border_info.chain_codes[j];
        }

        DetectedChar detectedChar;
        detectedChar.start = border_info.start_point.x;
        detectedChar.value = guessChainV1(sskode.str());
        interpretation.push_back(detectedChar);

        //ss << guessChainV1(sskode.str());
        // LOGD("CHAIN CODE: %s", sskode.str().c_str());
        // LOGD("KIRA KIRA = %d \n", match_chain_code(test,ukuran,numbers, numbers.size()));

    }


    stringstream ss;

    while(interpretation.size() > 0){
        int minValue = interpretation[0].start;
        int currentMin = 0;
        for (int i = 1; i < interpretation.size(); i++){
            if (interpretation[i].start < minValue){
                minValue = interpretation[i].start;
                currentMin = i;
            }
        }
        ss << interpretation[currentMin].value;
        interpretation.erase(interpretation.begin() + currentMin);
    }

    // [1] adalah ekspresi input, [2] adalah hasil perhitungan
    std::string tes[] = { ss.str().c_str(), "44" };
    jobjectArray hasil2 = createJavaArray(env, 2, tes);

    return hasil2;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//		VISION KEDUA



JNIEXPORT jobject JNICALL Java_com_ganesus_numbervision_Vision2_preProses (JNIEnv * env, jobject obj, jobject bitmap, jobject canvas){
    NativeBitmap* nCanvas = convertBitmapToNative(env, canvas);
    NativeBitmap* nativeBitmap = convertBitmapToNative (env, bitmap);
    NativeBitmap* pBitmap = grayscaleBitmap(nativeBitmap);
    delete  nativeBitmap;

    uint32_t nBitmapSize = pBitmap->bitmapInfo.height * pBitmap->bitmapInfo.width;
    uint32_t* histogram = createHistogram(pBitmap);
    uint32_t* color_transform = cumulative_equalization(histogram);

    NativeBitmap* gBitmap = transformNativeBitmap(pBitmap, color_transform);
    delete pBitmap;
    delete histogram;
    histogram = createHistogram(gBitmap);

    float otsu = generateOtsu(histogram, nBitmapSize);

    ARGB aGray;
    aGray.red = 127; aGray.green = 127; aGray.blue = 127; aGray.alpha = 255;
    uint32_t iGray= convertArgbToInt(aGray);

    for (uint32_t i = 0; i < nBitmapSize; i++){
        ARGB bitmapColor;
        convertIntToArgb(gBitmap->pixels[i], &bitmapColor);

        if (bitmapColor.red > otsu){
            nCanvas->pixels[i] = iGray;
        }
    }

    return convertNativeToBitmap(env, nCanvas);
}


JNIEXPORT jobjectArray JNICALL Java_com_ganesus_numbervision_Vision2_detectAll (JNIEnv * env, jobject obj, jobject bitmap, jstring path){
    NativeBitmap* nativeBitmap = convertBitmapToNative (env, bitmap);
    bool **image = convertToBoolmage(nativeBitmap);
    const char* path_knowledge = env->GetStringUTFChars( path , NULL ) ;
    vector<Knowledge> knowledge = createKnowledge(path_knowledge);

    vector<DetectedChar> interpretation;
    vector<BorderInfo> border_infos = get_border_infos(image,nativeBitmap->bitmapInfo.width,nativeBitmap->bitmapInfo.height);
    delete nativeBitmap;

    //FILE* file = fopen("/sdcard/textTest.txt","w+");

    for (int i=0;i<border_infos.size();i++) {
        BorderInfo border_info = border_infos[i];
        stringstream sskode;

        for (int j = 0; j < border_info.chain_codes.size(); j++) {
            sskode << border_info.chain_codes[j];
        }

        if (border_info.chain_codes.size() > TRESHOLD_ERROR) {
            DetectedChar detectedChar;
            detectedChar.start = border_info.start_point.x;
            detectedChar.value = guessChain(sskode.str(), knowledge);

            sskode << endl;
            detectedChar.chain = sskode.str();
            interpretation.push_back(detectedChar);
        }
        //ss << guessChain(sskode.str());
        //LOGD("CHAIN CODE: %s", sskode.str().c_str());

        //sskode << endl;
        //fputs(sskode.str().c_str(), file);

    }
    //fclose(file);

    stringstream ss;

    FILE* file = fopen("/sdcard/textTest.txt","w+");

    while(interpretation.size() > 0){
        int minValue = interpretation[0].start;
        int currentMin = 0;
        for (int i = 1; i < interpretation.size(); i++){
            if (interpretation[i].start < minValue){
                minValue = interpretation[i].start;
                currentMin = i;
            }
        }
        fputs(interpretation[currentMin].chain.c_str(), file);
        ss << interpretation[currentMin].value;
        interpretation.erase(interpretation.begin() + currentMin);

    }
    fclose(file);

    // [1] adalah ekspresi input, [2] adalah hasil perhitungan*/
    std::string tes[] = { ss.str().c_str(), "44" };
    //std::string tes[] = { ss.str().c_str(), "44" };
    jobjectArray hasil2 = createJavaArray(env, 2, tes);

    return hasil2;
}
