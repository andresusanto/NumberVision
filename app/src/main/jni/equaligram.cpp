// EQUALIGRAM - EQUALIZER INSTAGRAM
// Andre Susanto, M Yafi, Ramandika P, Kevin Yudi
// Pengcit - IF ITB

#include "fungsi.h"

#define NOT_SPECIFIED_COLOR -1

// supaya bisa dipanggil sama java
extern "C"
{
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
}


/////////////////////////////////////////////////////////////////////////////////////
// helper class and functions
/////////////////////////////////////////////////////////////////////////////////////




int adjustIntensity(int i,int old_min,int old_max,int new_min,int new_max){
	return (i - old_min)*(new_max - new_min)/(old_max - old_min) + new_min;
}

///////////////////////////// ALGORITMA 1 ///////////////////////////////////


uint32_t* get_cumulative_histogram(uint32_t* histogram) {

	uint32_t* cumulative_histogram = new uint32_t[256];

	int accumulation = 0;
	for (int i=0; i<256; i++) {
		accumulation += histogram[i];
		cumulative_histogram[i] = accumulation;
	}

	return cumulative_histogram;
}

int get_lower_bound(uint32_t* histogram) {
	for (int i=0;i<256;i++) {
		if (histogram[i] !=0) {
			return i;
		}
	}
	return NOT_SPECIFIED_COLOR;
}

int get_upper_bound(uint32_t* histogram) {
	for (int i= 255; i>=0; i--) {
		if (histogram[i] !=0) {
			return i;
		}
	}

	return NOT_SPECIFIED_COLOR;
}

uint32_t* cumulative_equalization(uint32_t* histogram) {

	uint32_t* color_transform = new uint32_t[256];
	uint32_t* cumulative_histogram = get_cumulative_histogram(histogram);

	int lower_bound = get_lower_bound(histogram);
	int upper_bound = get_upper_bound(histogram);

	for (int i=0; i<256; i++) {
		color_transform[i] = NOT_SPECIFIED_COLOR;
	}

	int total_pixels = cumulative_histogram[upper_bound];
	int denominator = total_pixels - cumulative_histogram[lower_bound];
	for (int i=lower_bound;i<=upper_bound;i++) {
		if (histogram[i])
			color_transform[i] = ((cumulative_histogram[i] - cumulative_histogram[lower_bound]) *
				(254) / denominator) + 1;
	}

	return color_transform;
}


//////////////////////// END ALGORITMA 1 //////////////////////////////////////


/////////////////////// ALGORITMA 2 /////////////////////////////////////////////


uint32_t* simple_equalization(uint32_t* histogram) {
	uint32_t* transform = new uint32_t[256];
	int min_histo = get_lower_bound(histogram);
	int max_histo = get_upper_bound(histogram);

	float scaler = 255.0f / (max_histo - min_histo);

	for (int i = min_histo; i < max_histo; i++)
		transform[i] = (i - min_histo) * scaler;

	return transform;
}


////////////////////// END ALGORITMA 2 //////////////////////////////////////////

/////////////////////// ALGORITMA STEP /////////////////////////////////////

uint32_t* equalize_step(uint32_t *in, uint32_t L){
	uint32_t* mapping = new uint32_t[256];
	float probability[256];
	uint32_t sum=0;
	for(int i=0;i<256;i++) sum+=in[i]; //total pixel freq
	for(int i=0;i<256;i++){
		probability[i]=(float)in[i]/sum;
		if(i>0) probability[i]+=probability[i-1]; //cdf function
		probability[i]=roundf(probability[i] * 1000) / 1000;
	}
	for(int i=0;i<256;i++){
		mapping[i]=(uint32_t)(L-1)*probability[i];
	}
	return mapping;
}

////////////////////////////// END STEP /////////////////////////////////////

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