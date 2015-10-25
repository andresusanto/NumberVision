#pragma once

#include "fungsi.h"
#include "ekualisasi.h"

#define STABILIZATION_FACTOR 4
#define TRESHOLD_ERROR 70

using namespace std;


Point get_next_traverse_point(Point current_black,Point current_traverse_point);
int get_chain_code(Point current_black,Point prev_black);
void erase_image(Point start_point, bool **image,int length,int height);
vector<int> get_chain_codes(Point start_point, bool **image,int length,int height);
Point get_start_point(bool **image, int length,int height);
vector<BorderInfo> get_border_infos(bool **image,int length,int height);



vector<ECode> stabileData(string original);
float calculateChain (string strKnowledge, string strTest );
vector<Knowledge> createKnowledgeV1();
char guessChainV1(string chainCode);


bool is_point(Point point,bool **image);
float generateOtsu(uint32_t* histogram, uint32_t total);
vector<Knowledge> createKnowledge(const char* path);
char guessChain(string chainCode, vector<Knowledge> knowledge);
bool** convertToBoolmage(NativeBitmap* nativeBitmap);
