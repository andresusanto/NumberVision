#pragma once

#include "fungsi.h"

#define STABILIZATION_FACTOR 4


using namespace std;


Point get_next_traverse_point(Point current_black,Point current_traverse_point);
int get_chain_code(Point current_black,Point prev_black);
void erase_image(Point start_point, bool **image,int length,int height);
vector<int> get_chain_codes(Point start_point, bool **image,int length,int height);
Point get_start_point(bool **image, int length,int height);
vector<BorderInfo> get_border_infos(bool **image,int length,int height);



vector<ECode> stabileData(string original);
float calculateChain (string strKnowledge, string strTest );
vector<Knowledge> createKnowledge();
char guessChain(string chainCode);
