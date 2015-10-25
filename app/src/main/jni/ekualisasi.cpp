#include "ekualisasi.h"

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
