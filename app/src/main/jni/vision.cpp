#include "vision.h"

Point get_next_traverse_point(Point current_black,Point current_traverse_point) {
    Point next = current_traverse_point;
    if (current_traverse_point.x == current_black.x-1 &&
        current_traverse_point.y == current_black.y+1) {
        next.y -= 1;
    } else if (current_traverse_point.x == current_black.x-1 &&
               current_traverse_point.y == current_black.y) {
        next.y -= 1;
    } else if (current_traverse_point.x == current_black.x-1 &&
               current_traverse_point.y == current_black.y-1) {
        next.x += 1;
    } else if (current_traverse_point.x == current_black.x &&
               current_traverse_point.y == current_black.y-1) {
        next.x += 1;
    } else if (current_traverse_point.x == current_black.x + 1 &&
               current_traverse_point.y == current_black.y-1) {
        next.y += 1;
    } else if (current_traverse_point.x == current_black.x + 1 &&
               current_traverse_point.y == current_black.y) {
        next.y += 1;
    } else if (current_traverse_point.x == current_black.x + 1 &&
               current_traverse_point.y == current_black.y + 1) {
        next.x -= 1;
    } else if (current_traverse_point.x == current_black.x &&
               current_traverse_point.y == current_black.y + 1) {
        next.x -= 1;
    }
    return next;
}

int get_chain_code(Point current_black,Point prev_black) {
    int chain_code = 0;
    if (prev_black.x == current_black.x-1 &&
        prev_black.y == current_black.y+1) {
        chain_code = 7;
    } else if (prev_black.x == current_black.x-1 &&
               prev_black.y == current_black.y) {
        chain_code = 0;
    } else if (prev_black.x == current_black.x-1 &&
               prev_black.y == current_black.y-1) {
        chain_code = 1;
    } else if (prev_black.x == current_black.x &&
               prev_black.y == current_black.y-1) {
        chain_code = 2;
    } else if (prev_black.x == current_black.x + 1 &&
               prev_black.y == current_black.y-1) {
        chain_code = 3;
    } else if (prev_black.x == current_black.x + 1 &&
               prev_black.y == current_black.y) {
        chain_code = 4;
    } else if (prev_black.x == current_black.x + 1 &&
               prev_black.y == current_black.y + 1) {
        chain_code = 5;
    } else if (prev_black.x == current_black.x &&
               prev_black.y == current_black.y + 1) {
        chain_code = 6;
    }
    return chain_code;
}

void erase_image(Point start_point,
                 bool **image,int length,int height) {

    Point direction[8] = {Point(1,0), Point(1,1), Point(0,1), Point(-1,1),
                          Point(-1,0), Point(-1,-1), Point(0,-1), Point(1,-1)};

    queue<Point> bfs_queue;
    bfs_queue.push(start_point);
    image[start_point.y][start_point.x] = 0;

    while(!bfs_queue.empty()) {
        Point front = bfs_queue.front();
        bfs_queue.pop();

        for (int i=0;i<8;i++) {
            Point current_point = front + direction[i];
            if (current_point.x >=0 && current_point.x <= length && current_point.y >= 0 && current_point.y <= height &&
                image[current_point.y][current_point.x]) {
                image[current_point.y][current_point.x] = 0;
                bfs_queue.push(current_point);
            }
        }
    }
}

vector<int> get_chain_codes(Point start_point,
                            bool **image,int length,int height) {

    vector<int> chain_codes;

    Point current_black = start_point;
    Point current_white = current_black;
    current_white.x -= 1;

    Point black0 = current_black;
    Point black1;

    bool has_first_found = false;

    Point traverse_point = current_white;
    Point traverse_point_prev = current_white;
    while (true) {


        traverse_point_prev = traverse_point;
        traverse_point = get_next_traverse_point(current_black, traverse_point);
        if (has_first_found && current_black == black0 && black1 == traverse_point) {
            break;
        }
        if (image[traverse_point.y][traverse_point.x]) {
            int chain_code = get_chain_code(traverse_point,current_black);

            if (!has_first_found) {
                black1 = traverse_point;
                has_first_found = true;
            }

            chain_codes.push_back(chain_code);
            current_black = traverse_point;
            current_white = traverse_point_prev;
            traverse_point = current_white;

        }
    }

    return chain_codes;

}

Point get_start_point(bool **image, int length,int height) {
    for (int i=0;i<height;i++) {
        for (int j=0;j<length;j++) {
            if (image[i][j]) return Point(j,i);
        }
    }
    return Point(-1,-1);
}

vector<BorderInfo> get_border_infos(bool **image,int length,int height) {
    vector<BorderInfo> border_infos;
    while(true) {
        Point start_point = get_start_point(image, length, height);
        if (start_point.x == -1 && start_point.y == -1) {
            break;
        }
        BorderInfo border_info;
        border_info.start_point = start_point;
        border_info.chain_codes = get_chain_codes(start_point, image, length, height);
        border_infos.push_back(border_info);
        erase_image(start_point,image,length,height);
    }
    return border_infos;
}



vector<ECode> stabileData(string original){
    vector<ECode> training;

    char currentDirection = original[0];
    int currentValue = 1;
    for (int i = 1 ; i < original.size(); i++){
        if (currentDirection == original[i])
            currentValue++;
        else{
            ECode kode;
            kode.direction = currentDirection;
            kode.value = currentValue;

            training.push_back(kode);

            currentValue = 1;
            currentDirection = original[i];
        }
    }
    ECode kode;
    kode.direction = currentDirection;
    kode.value = currentValue;
    training.push_back(kode);

    // pemotong chain code, kalo ga dipake malah hasilnya lebih bagus :/
    /*
    for (int i = 1; i < training.size() - 1; i++){
        if (training[i - 1].direction == training[i + 1].direction && training[i - 1].value + training[i + 1].value > STABILIZATION_FACTOR && training[i].value == 1){
            training[i - 1].value = training[i - 1].value + training[i].value + training[i + 1].value;
            training.erase(training.begin() + i, training.begin() + i + 2);
            //cout << (i-1) << " After Train : " << training[i - 1].value << endl;
            i--;
        }
    }*/

    return training;
}

float calculateChain (string strKnowledge, string strTest ){
    vector<ECode> knowledge, test;
    vector<ECode> data1 = stabileData (strKnowledge);
    vector<ECode> data2 = stabileData(strTest);

    // pilih yang paling besar sebagai basis (mengandung paling banyak error)
    if (data1.size() > data2.size()){
        knowledge = data1;
        test = data2;
    }else{
        knowledge = data2;
        test = data1;
    }

    int testSize = test.size(), knowledgeSize = knowledge.size();
    int knowledgeChains = 0, testChains = 0;

    for (int i = 0 ; i < knowledgeSize; i++)
        knowledgeChains += knowledge[i].value;

    for (int i = 0 ; i < testSize; i++)
        testChains += test[i].value;


    float currentScore = 0; int iteratorKnowledge = 0;

    for (int i = 0; i < testSize && iteratorKnowledge < knowledgeSize; i++){
        if (test[i].direction == knowledge[iteratorKnowledge].direction){
            currentScore += ((float)test[i].value / testChains + (float)knowledge[iteratorKnowledge].value / knowledgeChains) * 2.0f;
        }else{
            currentScore -= (float)knowledge[iteratorKnowledge].value / (knowledgeChains * 2.0f);
            i--;
        }
        iteratorKnowledge++;

        if (iteratorKnowledge == knowledgeSize){
            for (; i < testSize; i++){
                currentScore -= (float)test[i].value / testChains;
            }
        }
    }


    return currentScore;
}

vector<Knowledge> createKnowledgeV1(){
    vector<Knowledge> result;

    Knowledge satu, dua, tiga, empat, lima, enam, tujuh, delapan, sembilan, nol;


    satu.meaning = '1';
    dua.meaning = '2';
    tiga.meaning = '3';
    empat.meaning = '4';
    lima.meaning = '5';
    enam.meaning = '6';
    tujuh.meaning = '7';
    delapan.meaning = '8';
    sembilan.meaning = '9';
    nol.meaning = '0';

    satu.data = "0000000122222222222222222222222222222222222222222222222222222100000000000222224444444444444444444444444444444444466666000000000000076666666666666666666666666666666666666666666665334343433434466666770770707707707";
    dua.data = "00000000000100101011121212222222222222323232323323333233333333333333310000000000000000000000000012222244444444444444444444444444444444444445666667677777777777767777776777676767666766666656565554454444434434334344466666607707007007";
    tiga.data = "000000000001001010111212212222222232233233434331001010111121221222222322323233343434344434444444444445444544545566666600010101010010000000700770767766666666565555454445444444444445666700000000007007707767676666656655554544444434434343344466666607707070007";
    empat.data = "000000000012222222222222222222222222222222222222210000000122223444444432222222222222444444446666666666666544444444444444444444444445666666767677676776767767677676776767767677677";
    lima.data = "00000000000000000000000000000002222224444444444444444444444432222222222222222100000000000010001010110211212212222222223223232333343434344434444444444445444544546566667001010010010000000700707777766766666665665555454445444444444434445466666666666666666666666666667";
    enam.data = "00000000000010001122224444544454444444343433333232323222232222170070070000000000001001011021121222122222222322232323333343434434444444444454445455556556656665666666566666666666766666676667667676777777707070007";
    tujuh.data = "000000000000000000000000000000000000001222223223232323223232322323232232323223232323223232322323234444444457676767667676767667676767667676767667676767667676767544444444444444444444444444444566667";
    delapan.data = "0000000000010001010112112221222322232333333433210110111021121221222223223233234343434443444444444444454445454555565665666667667676770777070765545554655656656666766676777707070007";
    sembilan.data = "000000000001001010111212121221222222122222222222223222223222323223323333334344344344444444444454445456666600001000100000007007077777767676667666665343444344444444444544454555556656665666666676667676777770707007";
    nol.data = "000000000001001010111121212212221222222222222222222222222322223223233233334343443444444444445445454555565656656666566666666666666666666666766676676767677770707007";

    result.push_back(satu);
    result.push_back(dua);
    result.push_back(tiga);
    result.push_back(empat);
    result.push_back(lima);
    result.push_back(enam);
    result.push_back(tujuh);
    result.push_back(delapan);
    result.push_back(sembilan);
    result.push_back(nol);

    return result;
}

char guessChainV1(string chainCode){
    vector<Knowledge> knowledge = createKnowledgeV1();

    char currentChar = knowledge[0].meaning;
    float currentMax = calculateChain(knowledge[0].data, chainCode);
    for (int i = 1 ; i < knowledge.size(); i++){
        float rate = calculateChain(knowledge[i].data, chainCode);
        if (rate > currentMax){
            currentMax = rate;
            currentChar = knowledge[i].meaning;
        }
    }

    return currentChar;
}

vector<Knowledge> createKnowledge(const char* path){
    vector<Knowledge> result;
    LOGD("Loaded Knowledge File %s", path);
    ifstream infile(path);

    char tmp_char; string tmp_string;
    while (infile >> tmp_char >> tmp_string)
    {
        Knowledge tmp_knowledge;
        tmp_knowledge.meaning = tmp_char;
        tmp_knowledge.data = tmp_string;

        result.push_back(tmp_knowledge);
    }

    return result;
}

char guessChain(string chainCode, vector<Knowledge> knowledge){


    char currentChar = knowledge[0].meaning;
    float currentMax = calculateChain(knowledge[0].data, chainCode);
    for (int i = 1 ; i < knowledge.size(); i++){
        float rate = calculateChain(knowledge[i].data, chainCode);
        if (rate > currentMax){
            currentMax = rate;
            currentChar = knowledge[i].meaning;
        }
    }

    return currentChar;
}


bool is_point(Point point,bool **image) {

    int x = point.x;
    int y = point.y;

    return image[y][x] && !image[y-1][x] && !image[y][x-1] && !image[y+1][x] && !image[y][x+1];
}

float generateOtsu(uint32_t* histogram, uint32_t total) {
    int sum = 0;
    for (int i=1;i<256; ++i) sum+= i *histogram[i];

    int sumB = 0;
    int wB = 0;
    int wF = 0;
    int mB = 0;
    int mF = 0;
    float max = 0.0f;
    float between = 0.0f;
    float threshold1 = 0.0f;
    float threshold2 = 0.0f;

    for (int i=0;i<256;++i) {
        wB += histogram[i];
        if (wB == 0) continue;
        wF = total - wB;
        if (wF == 0) break;

        sumB += i * histogram[i];

        mB = sumB / wB;
        mF = (sum - sumB) /wF;

        between = wB * wF * (mB - mF) * (mB - mF);
        if (between >= max) {
            threshold1 = i;
            if ( between > max ) {
                threshold2 = i;
            }
            max = between;
        }
    }

    return (threshold1 + threshold2) / 2.0f;
}


bool** convertToBoolmage(NativeBitmap* nativeBitmap){
    bool** image = new bool*[nativeBitmap->bitmapInfo.height];
    NativeBitmap* pBitmap = grayscaleBitmap(nativeBitmap);

    uint32_t nBitmapSize = pBitmap->bitmapInfo.height * pBitmap->bitmapInfo.width;
    uint32_t* histogram = createHistogram(pBitmap);
    uint32_t* color_transform = cumulative_equalization(histogram);

    NativeBitmap* gBitmap = transformNativeBitmap(pBitmap, color_transform);
    delete pBitmap;

    histogram = createHistogram(gBitmap);

    float otsu = generateOtsu(histogram, nBitmapSize);

    //FILE* file = fopen("/sdcard/textTest.txt","w+");

    //stringstream ss;


    for (int i=0;i<gBitmap->bitmapInfo.height;i++) {
        image[i] = new bool[gBitmap->bitmapInfo.width];
        for (int j=0;j<gBitmap->bitmapInfo.width;j++) {
            ARGB warna;
            convertIntToArgb(gBitmap->pixels[i * gBitmap->bitmapInfo.width + j], &warna);

            image[i][j] = (warna.red > otsu);


            //if (warna.red > otsu)
            //    ss << "image[" << i << "][" << j << "] = true;\n"; //LOGD("image[%d][%d] = true;", i, j);
            //else
            //    ss << "image[" << i << "][" << j << "] = false;\n"; //LOGD("image[%d][%d] = false;", i, j);

            //fputs(ss.str().c_str(), file);
            //ss.str(string());
        }
    }
    //fclose(file);

    for (int i=0;i<gBitmap->bitmapInfo.width;i++) {
        image[0][i] = false;
        image[gBitmap->bitmapInfo.height-1][i] = false;
    }

    for (int i=0;i<gBitmap->bitmapInfo.height;i++) {
        image[i][0] = false;
        image[i][gBitmap->bitmapInfo.width-1] = false;
    }

    delete gBitmap;

    return image;
}
