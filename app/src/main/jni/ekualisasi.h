#pragma once

#include "fungsi.h"

#define NOT_SPECIFIED_COLOR -1

using namespace std;

int adjustIntensity(int i,int old_min,int old_max,int new_min,int new_max);
uint32_t* get_cumulative_histogram(uint32_t* histogram);
int get_lower_bound(uint32_t* histogram);
int get_upper_bound(uint32_t* histogram);

uint32_t* cumulative_equalization(uint32_t* histogram);
uint32_t* simple_equalization(uint32_t* histogram);
uint32_t* equalize_step(uint32_t *in, uint32_t L);
