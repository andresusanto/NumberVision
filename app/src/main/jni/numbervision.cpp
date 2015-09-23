// EQUALIGRAM - EQUALIZER INSTAGRAM
// Andre Susanto, M Yafi, Ramandika P, Kevin Yudi
// Pengcit - IF ITB

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


#define  LOG_TAG    "NUMBER VISION"
#define  LOGD(...)  __android_log_print(ANDROID_LOG_DEBUG,LOG_TAG,__VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)

#define STABILIZATION_FACTOR 4 // faktor untuk stabilisasi (copot noise dari chain code)
#define NOT_SPECIFIED_COLOR -1
#define TRESHOLD_ERROR 70

// supaya bisa dipanggil sama java
extern "C"
{
    JNIEXPORT jobjectArray JNICALL Java_com_ganesus_numbervision_MainActivity_detectAll (JNIEnv * env, jobject obj, jobject bitmap);
    JNIEXPORT jobject JNICALL Java_com_ganesus_numbervision_MainActivity_preProses (JNIEnv * env, jobject obj, jobject bitmap, jobject canvas);
}


using namespace std;

/////////////////////////////////////////////////////////////////////////////////////
// helper class and functions
/////////////////////////////////////////////////////////////////////////////////////
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

typedef struct point{
    int x;
    int y;

    point() : x(0) , y(0) {}
    point(int x,int y) : x(x), y(y) {}
} Point;


typedef struct border_info{
    Point start_point;
    vector<int> chain_codes;
}BorderInfo;

class NativeBitmap{
public:
    uint32_t* pixels;
    AndroidBitmapInfo bitmapInfo;

    NativeBitmap(){
        pixels = NULL;
    }

    NativeBitmap(NativeBitmap* nBitmap){
        this->bitmapInfo = nBitmap->bitmapInfo;
        uint32_t pixelsCount = this->bitmapInfo.height * this->bitmapInfo.width;

        this->pixels = new uint32_t[pixelsCount];
        memcpy(this->pixels, nBitmap->pixels, sizeof(uint32_t) * pixelsCount);
    }
};


typedef struct{
    uint8_t alpha, red, green, blue;
} ARGB;

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

jobjectArray createJavaArray(JNIEnv *env, jsize count, std::string elements[]){
    jclass stringClass = env->FindClass("java/lang/String");
    jobjectArray row = env->NewObjectArray(count, stringClass, env->NewStringUTF(""));
    jsize i;

    for (i = 0; i < count; i++) {
        env->SetObjectArrayElement( row, i, env->NewStringUTF(elements[i].c_str()));
    }
    return row;
}

bool operator==(const Point& lhs, const Point& rhs)
{
    return (lhs.x == rhs.x) && (lhs.y == rhs.y);
}

Point operator+(const Point& lhs, const Point& rhs)
{
    return Point(lhs.x + rhs.x , lhs.y + rhs.y);
}


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

bool is_point(Point point,bool **image) {

    int x = point.x;
    int y = point.y;

    return image[y][x] && !image[y-1][x] && !image[y][x-1] && !image[y+1][x] && !image[y][x+1];
}

vector<int> get_chain_codes(Point start_point,
                            bool **image,int length,int height) {

    vector<int> chain_codes;

    if (is_point(start_point,image)) return chain_codes;

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

////////////////////////////////////////////////////////////////////////////////////////
// ALGORITMA MATCHER
////////////////////////////////////////////////////////////////////////////////////////


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

vector<Knowledge> createKnowledge(){
    vector<Knowledge> result;

    Knowledge satu, dua, tiga, empat, lima, enam, tujuh, delapan, sembilan, nol, karA, karN, karD, karR, karE;
    Knowledge plat1B5, platA, plat3R, plat3F, plat3P, plat13, plat32, plat30, plat38, plat39, plat21, plat212, plat20,plat201, plat202, plat132, plat14, plat142, plat19, plat11, plat2B, plat1B2, plat1H, plat1H2, plat1Q, plat1B, plat1B3, plat1B4;

    plat2B.meaning = 'B';
    plat1B2.meaning = 'B';
    plat1B.meaning = 'B';
    plat1B3.meaning = 'B';
    plat1B4.meaning = 'B';
    plat1B5.meaning = 'B';
    plat2B.data = "100010751710000010111025543111211121221222122223222323233323331100011212124655321212222122222232232233233343344344444434444544444444444445444656666666666666666666666666666666666666666666666666666666666666666666567667000000000000000007";
    plat1B4.data = "010007001007100000010010011011011111121122121222122222222212222222232222223222323233233332111112112212122221222222222212222222322222232232232332333334334343443444344444444444444444444444444454556566666666666666666666666666666575765666666666666666666666666666666666666666666666666666666666666566666756666666666666666666777000000000007";
    plat1B.data = "00000000000000000000000001000101101021012111121221222212222222222222222232222312323223233233343311102111212212212222122222222222222232222222322322323323333343433444344434444444444444444444444444445566666666666675666666666666666666666666666666756666666675666666666666666666666666766666666666666666666676566675766666666667670007";
    plat1B2.data = "100000000000000000000000000001000101011011111121121212221222222231132222222222222223222323323333332101111211212212221222312222222123221322232222223223233324333334343434443444444444444444444444535435543556666666666666666566666666666666675666667656666666666666666666666666666666666666666665756766666666666666566666666666666666667707";
    plat1B3.data = "000170000000000000010010010101110111211212122212222212222222222222232222222323223233233334311021112121212212222222212222222222232222222232232323323343243334343443444344444444444444444444444444545566566666666666765667666666666666666666666666666666666666666666666666666666667666666666666666666666666666666666676666666666777000000007";
    plat1B5.data = "101000007170000000000000000100110111211212122122222322232233233323322212111212212222222222323232323334333434443444444444444444444444444565676666666666666666676666666666666666667666666666666666666666676666666665607";

    plat32.meaning = '2';
    plat30.meaning = '0';
    plat30.data = "000001000101010111121121221222212222222222221222222222222222222222222222222222222222222222223223232333333434443444444444454454545545565565665666665666666666666666666666665666666666666666666666666666666667666676677677777007070007";
    plat32.data = "0001000100101101211121212222122222322223223232323232332323233232323323323233232332332323232322110000000000000000000000000000001222222224444444444444444444444444444444444444444444656666666667676767767677676776767677676776767677676767676776767676676766666665665555545444534444343243223223224444444466666667676777677070707000007";


    plat38.meaning = '8';
    plat38.data = "0000000000000100101111112121221222222222222222222222322232232332121121221222122222222222222222322223223333334344344434444544445454545555656565666566666666666666676667667676776655655656656666666566666666666676667667677760777007";

    plat39.meaning = '9';
    plat39.data = "00000000100010011111112122122222131222222222222222222222222222222223222222222222222222222232223223323334334434434444445444545455455655665666666600000002122212111010100007007777767666676666666666553444444444444445445455455565566566666566666666666666666766666666676676677677770707007";

    plat3R.meaning = 'R';
    plat3R.data = "000000000000000000010000010101011121121212222122222222222222322223232323332332221222122221222212222122221222122212221222224444444465666566665666656666566656666566656665666655444444444444322222222222222322222222222222222222222222224444444546666666666766666666666666666666666666666666666666666666666666666666666766666666666666666666667";

    plat3F.meaning = 'F';
    plat3F.data = "0000000000000000000000000000000000000122222224444444444444444444444444444432222222222222222222223222222221210100000000000000000000000000123222224444444444444444444444444444323222222222222222222322222222222222222224444444466666666666666666666666666666676666666666666666666666666666666666666666676666666666666666666";

    plat3P.meaning = 'P';
    plat3P.data = "000000000000000000000001000010101111121221221222222222222322222323223333334343444443444444444444322222222222232222222222222222222222222223244443456667666666666666666666666666666666666666666666666666666666666666666666666666666666666666666";


    plat13.meaning = '3';
    plat132.meaning = '3';
    plat13.data = "00000170000000001001010101111111121212122212222212223123222222223222322322323233333321110211212121222212222222222221322222232222232232323233334324334343434444344534454444454445455455455565556655666656666666770700001021223122212112102101010100001070107700777707676767666667666666667566756775436655655554545445444445456666666770007010700770777776760757576666666675766665666666546555455545445444444443443343443133223222223233443445455675676666766767767776077707700707";
    plat132.data = "00000000000000100101010111021111121212212221222222222222222222222322232232233233323432101211211221212222122222222222222222222322223223232332333334334334434434443444444454444545445545545555655656566566657566676707070010112122222212111111101001000000070070777707676766667666666666666666665666556555545454444544445656666677607000007007070777767676676666666765667566666665655655545545444444444443433433332322322222333434445455656666766676676767777777707070707007";

    plat1H.meaning = 'H';
    plat1H2.meaning = 'H';
    plat1H2.data = "00101121222222222222222222222222222222221231222222222222221000000000000000000000000000000000766656666666666666666666666666666666666666666666666677070100011221222222222222222222222222222222222132222222122222222222222222222222222222222222222222222222222222212312222222222222222222334344444556566666666666666666666666666666666666576665666666666666666544444444444444444444444454344544332222122223212222222222222222222222222222222222222222222232343444445455566667666666666666666666666666666666666666666666666665756666765666766665666666666666666666666666666666666666666666666666666670707";
    plat1H.data = "1001221222222222222221222222222222222222222313122222223221000071000000000000000000000001000707666666666666666666666666666666666666666666666657567677001000212213122222222222222222122222223122223132222213122222222222222222222222222222222123123122223122223132222221321222222222222233434544556666666666666666567656675666756766666666666567566666666666544444453543543544435443454443454343222222222222222222222222222222222222222222222222222222222233444444455566667666666666666666666576576666666666666666666666665666666666666666666666666666666666666666666666666666666666657657566667670007";

    plat14.meaning = '4';
    plat142.meaning = '4';
    plat142.data = "00010112122323232233232232323223323223232323232232323232323232232323232323223232323232323223232322100000000000000017100000076666666666766707000000111222222222222100001122222222233444432222222222222222232334434454556566666666666666666654344454444435444444444444444444444455566666666666666667676766767676767676676767676767667676767676767676676767676767676766767676767676767676767007";
    plat14.data = "000000112123222323232323232322323232323232431323232232323232323232322323232322323232322323232323232244310000000000000000000000000000077566660754766667070000101222222222231210000021123222222345435322222222222222222223234434653455666666666666666666654444444534444444444444444444444454454565666666675766667776666767676767676676767667676767676676767676767676776766767676766767676767676767667677";

    plat19.meaning = '9';
    plat19.data = "017001000010010101101111111212112212222212222222222322222222222222222222222222222222222222222222222222222222222222232213222323223233233333433344354314434444444454444454544455454555556565565665665756666760707000101222222212211310101110101000100007000077707607676766766666666666667566757666666665075666654343434434444435354444446535445454554654556556565656666566666666666666676666666666766676767677677077707707007000007";

    platA.meaning = 'A';
    platA.data = "00001243111222122122221222122221222122221221222212222122221222122221222212221222233444454566566656666566554454444444344332322223222232343444454656766667666667666766766666766676667666766667666676666676666766676667666070000756";


    plat11.meaning = '1';
    plat21.meaning = '1';
    plat212.meaning = '1';
    plat212.data = "00002101012122322222222222222222232222222222222222223222222222222222223222222222323444444566666666666666666666676666666666666666667666666666666666654444445566667667544313576077707070707";
    plat21.data = "00000132122222222222222222222222232222222222222222222222222222222222222232123213233444445566666666666666666666666666666675676666666666666666666666665444345465666667607770070700775";
    plat11.data = "000001121222222222222222222222222222222123213213222222222212222222222222222222222222222222122223212222222232222222222212222222222232343444545566666666666666665766656765766666666666666566667656666666666666666666666665766666656676666666666666666656666667665344445466656677770070707";

    plat1Q.data = "107017100001001010011101111121121212221122222222212222222312222222222222222222222122222222222222222222222222222222222223222322332443104532111121212654312134344454656544434434445344543444754345445545555555565656565656676665326767567566675475666666666666666666665765666666666666666676655676666666666666676666766776767777707707070007";
    plat20.data = "0010000101031101111111212122212222232222222222222222222222222222222222232222323233333334343444444444444444545455455655665656666566667666666666666666666666666666666666667667667767767077070076607007";
    plat201.data = "00171000021010001101111111212122212322222222222222222222222222232222213122322322323323334334343434444444544444445455455565566566666566666766666666666666667666666666666666666676676767777707076534434376717707007";
    plat202.data = "100001010110111121212221222222223222222222222222222222222222232222232232332333334334434443444544445454545554656556566665666666766666666666666666666666667666666676766776607767700700700700007";
    plat1Q.meaning = 'Q';
    plat20.meaning = '0';
    plat201.meaning = '0';
    plat202.meaning = '0';


/*

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

    karA.meaning = 'A';
    karN.meaning = 'N';
    karD.meaning = 'D';
    karR.meaning = 'R';
    karE.meaning = 'E';

    karA.data = "00000000000000000000000001000111122122121221221221212212212212212122122122121221221221212212212212122122122122121221221221212212212212122122122122121221221221212212212212122122122122121221221221212212212212122122122121221221221221212212212212122122212222324434444444444444444444444454556566566566566566565665665665665665656656656656656566566565444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444433223223223223223223232232232232232232232232232232232232323344344444444444444444444454656666766676676676766766766766767667667667676676676676766766766767667667667667676676676676766766766767667667667676676676676676766766766767667667667676676676676766766766766767667667667676676676676766766766767667667667667676677607007";
    karN.data = "000000000000001000000001010010121111121121121212121211212121212121121212121212121121212121212112121212121211212121212121121212121212121121212121212112121212121212112121212121212121212121212121212121212121211212121766666666666666666666666666666666666666666666666666666666566666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666667677007000000000000000000100112222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222232233343344444444444444444444444545454555555655655655656565656565565656565656556565656565655656565656565565656565656556565656565655656565656565565656565656556565656565655656565656565565656565656565655656565656565656565656565665656532222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222223344344444444444444444454455656666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666667666667677770700007";
    karD.data = "000000000000000000000000000000000000000000000000000100000000000000000100000000100001000010001001001001010101010101010101011110111101111111211121111211122121212121212122121222122212221222122212222222222212222222222222222222222222222322222222222232223222322223222322232232232323232322323232332333323332333324333343334333343343434434343434343443443444344344444344443444444443444444444444444444444444444444444444444444444444444444444444444444444444545556566666566666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666766666776707700007";
    karR.data = "00000000000000000000000000000000000000000000000100000000000000000001000000100001000100100101001010110101101111111112111212121212122122212221222222222222222222222223222322232322323232332332333333343343334343433434434344343101011011011111111121121121211212121212212121212212122121212212122121221212212122121212212122121221221212221222233443444444444444444444444545556565665656656656566565665656656566565665665656656566565665656565656656565655656555565555545554545544544544454444444445444444444444444444443222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222243443444444444444444444544546656666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666766666776707700007";
    karE.data = "00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000010011221222222222222223233444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444322222222222222222222222222222222222222222222222222222222222222222222222100000000000000000000000000000000000000000000000000000000000000000000000000000100112122222222222222232334444444444444444444444444444444444444444444444444444444444444444444444444444444432222222222222222222222222222222222222222222222222222222222222222222222222222222221000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000010011212222222222222223233444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444545556566666566666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666766666776707700007";

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

    /*result.push_back(satu);
    result.push_back(dua);
    result.push_back(tiga);
    result.push_back(empat);
    result.push_back(lima);
    result.push_back(enam);
    result.push_back(tujuh);
    result.push_back(delapan);
    result.push_back(sembilan);
    result.push_back(nol);

    result.push_back(karA);
    result.push_back(karD);
    result.push_back(karR);
    result.push_back(karN);
    result.push_back(karE);*/

    result.push_back(plat13);
    result.push_back(plat132);
    result.push_back(plat14);
    result.push_back(plat142);
    result.push_back(plat19);
    result.push_back(plat11);
    result.push_back(plat2B);
    result.push_back(plat1B2);
    result.push_back(plat1B3);
    result.push_back(plat1B4);
    result.push_back(plat1H);
    result.push_back(plat1H2);
    result.push_back(plat1Q);
    result.push_back(plat1B);
    result.push_back(platA);
    result.push_back(plat20);
    result.push_back(plat201);
    result.push_back(plat202);
    result.push_back(plat212);
    result.push_back(plat21);
    result.push_back(plat3R);
    result.push_back(plat3F);
    result.push_back(plat3P);
    result.push_back(plat32);
    result.push_back(plat38);
    result.push_back(plat39);
    result.push_back(plat30);
    result.push_back(plat1B5);


    return result;
}

char guessChain(string chainCode){
    vector<Knowledge> knowledge = createKnowledge();

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
///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////

JNIEXPORT jobject JNICALL Java_com_ganesus_numbervision_MainActivity_preProses (JNIEnv * env, jobject obj, jobject bitmap, jobject canvas){
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


JNIEXPORT jobjectArray JNICALL Java_com_ganesus_numbervision_MainActivity_detectAll (JNIEnv * env, jobject obj, jobject bitmap){
    NativeBitmap* nativeBitmap = convertBitmapToNative (env, bitmap);
    bool **image = convertToBoolmage(nativeBitmap);

    /*image = new bool*[nativeBitmap->bitmapInfo.height];
    for (int i=0;i<nativeBitmap->bitmapInfo.height;i++) {
        image[i] = new bool[nativeBitmap->bitmapInfo.width];
        for (int j=0;j<nativeBitmap->bitmapInfo.width;j++) {
            ARGB warna;
            convertIntToArgb(nativeBitmap->pixels[i * nativeBitmap->bitmapInfo.width + j], &warna);

            image[i][j] = !(warna.blue == 255 && warna.green == 255 && warna.red == 255);
        }
    }*/

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
            detectedChar.value = guessChain(sskode.str());

            sskode << endl;
            detectedChar.chain = sskode.str();
            interpretation.push_back(detectedChar);
        }
        //ss << guessChain(sskode.str());
        LOGD("CHAIN CODE: %s", sskode.str().c_str());

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