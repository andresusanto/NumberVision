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
#include <fstream>


#define  LOG_TAG    "NUMBER VISION"
#define  LOGD(...)  __android_log_print(ANDROID_LOG_DEBUG,LOG_TAG,__VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)

#define STABILIZATION_FACTOR 4 // faktor untuk stabilisasi (copot noise dari chain code)
#define NOT_SPECIFIED_COLOR -1
#define TRESHOLD_ERROR 70

#define E '0'
#define S '2'
#define W '4'
#define N '6'
#define RIGHT 'R'
#define LEFT 'L'

//subchain code data yang diambil
#define LENGTH_TAKE 30
//derajat minimal agar dia dikatakan belok
#define MIN_D 50.0
//derajat maximal agar dia dikatakan belok
#define MAX_D 90.0
//maximal banyak kode belok
#define MAX_TURN_CODE 50

// supaya bisa dipanggil sama java
extern "C"
{
    JNIEXPORT jobjectArray JNICALL Java_com_ganesus_numbervision_MainActivity_detectAll (JNIEnv * env, jobject obj, jobject bitmap, jstring path);
    JNIEXPORT jobject JNICALL Java_com_ganesus_numbervision_MainActivity_preProses (JNIEnv * env, jobject obj, jobject bitmap, jobject canvas);
    JNIEXPORT jobjectArray JNICALL Java_com_ganesus_numbervision_MainActivity_detectMerek (JNIEnv * env, jobject obj, jobject bitmap);
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

struct Train {
    char label;
    string path;
    Train(char label,string path){
        this->label = label;
        this->path = path;
    }
};

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


//////////////////////////////////////////////////////////////////////////////////////
/// ALGORITMA BELOK BELOK
//////////////////////////////////////////////////////////////////////////////////////

double degree_by_vote(const string &test){
    double dx[] = { 1, 1, 0,-1,-1,-1, 0, 1};
    double dy[] = { 0,-1,-1,-1, 0, 1, 1, 1};

    double x = 0;
    double y = 0;
    for(int i = 0; i < (int) test.size(); i++){
        int _dir = (int) (test[i] - '0');
        x += dx[_dir];
        y += dy[_dir];
        //printf("%c %.3lf\n",test[i],deg);
    }
    double temp = atan2 (-y,x) * 180 / 3.14159265;
    temp += 360.;
    if (temp > 360.0) temp -= 360.0;
    return temp;
}


bool is_right_turn(double s,double d){
    double diff = fabs(s - d);
    if (diff > 180.0) diff = 360.0 - diff;
    if (diff < MIN_D || diff > MAX_D) return false;

    s = s + diff;
    if (s > 360.0) s -= 360.0;
    return fabs(s - d) <= 0.0001;
}

bool is_left_turn(double s,double d){
    double diff = fabs(s - d);
    if (diff > 180.0) diff = 360.0 - diff;
    if (diff < MIN_D || diff > MAX_D) return false;

    d = d + diff;
    if (d > 360.0) d -= 360.0;
    return fabs(s - d) <= 0.0001;
}

string generate_turn(const string &code){
    string ret;
    for(int i = 0; i + LENGTH_TAKE < (int) code.size(); ){
        string sleft = code.substr(i,LENGTH_TAKE/2);
        string sright = code.substr(i + (LENGTH_TAKE/2),LENGTH_TAKE/2);
        double dl = degree_by_vote(sleft);
        double dr = degree_by_vote(sright);
        double diff = fabs(dl - dr);
        if (diff > 180.0) diff = 360. - diff;
        if (is_left_turn(dl,dr)){
            ret += LEFT;
            i += LENGTH_TAKE/2;
            printf("got left turn at %d, %s %s, deg = %.3lf %.3lf %.3lf\n",i,sleft.c_str(),sright.c_str(),dl,dr,diff);
        } else if (is_right_turn(dl,dr)){
            ret += RIGHT;
            i += LENGTH_TAKE/2;
            printf("got right turn at %d, %s %s , deg = %.3lf %.3lf %.3lf\n",i,sleft.c_str(),sright.c_str(),dl,dr,diff);
        } else {
            i++;
        }
    }
    return ret;
}


int edit_distance(const string& a,const string& b){
    int dp[MAX_TURN_CODE][MAX_TURN_CODE];

    for(int ia = 1; ia <= (int) a.size(); ia++){
        dp[ia][0] = ia;

    }
    for(int ib = 1; ib <= (int) b.size(); ib++){
        dp[0][ib] = ib;
    }

    for(int ia = 1; ia <= (int) a.size(); ia++){
        for(int ib = 1; ib <= (int) b.size(); ib++){
            if (a[ia - 1] == b[ib- 1]){
                dp[ia][ib] = dp[ia-1][ib-1];
            } else {
                //b gak boleh dihapus
                dp[ia][ib] = min(dp[ia-1][ib],min(dp[ia-1][ib-1],dp[ia][ib-1])) + 1;
            }
        }
    }
    return dp[a.size()][b.size()];
}

vector<char> predict(const string& chain_code,const vector<Train>& trains){
    vector<char> probabilities;
    probabilities.clear();

    int cost = edit_distance(chain_code,trains[0].path);
    probabilities.push_back(trains[0].label);
    for(int i = 1; i < (int) trains.size(); i++){
        int cnow = edit_distance(chain_code,trains[i].path);
        if (cnow < cost){
            cost = cnow;
            probabilities.clear();
            probabilities.push_back(trains[i].label);
        } else if (cnow == cost){
            probabilities.push_back(trains[i].label);
        }
    }
    return probabilities;
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
        //result.push_back(Train(tmp_char, tmp_string));
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

///////////////////////////////////////////////////////////////////////////////////
// VOTING?
//////////////////////////////////////////////////////////////////////////////////


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


JNIEXPORT jobjectArray JNICALL Java_com_ganesus_numbervision_MainActivity_detectAll (JNIEnv * env, jobject obj, jobject bitmap, jstring path){
    NativeBitmap* nativeBitmap = convertBitmapToNative (env, bitmap);
    bool **image = convertToBoolmage(nativeBitmap);
    const char* path_knowledge = env->GetStringUTFChars( path , NULL ) ;
    vector<Knowledge> knowledge = createKnowledge(path_knowledge);
    //vector<Train> train = createKnowledge(path_knowledge);

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
            string turns = generate_turn(sskode.str());

            //vector<char> predictions = predict(turns, train);

            DetectedChar detectedChar;
            detectedChar.start = border_info.start_point.x;
            detectedChar.value = guessChain(sskode.str(), knowledge);

            sskode << endl;
            detectedChar.chain = sskode.str();
            interpretation.push_back(detectedChar);

            //LOGD("%d, %d CHAIN CODE: %s", border_info.start_point.x, border_info.start_point.y, sskode.str().c_str());
            //LOGD("%s", turns.c_str());

        }
        //ss << guessChain(sskode.str());


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

JNIEXPORT jobjectArray JNICALL Java_com_ganesus_numbervision_MainActivity_detectMerek (JNIEnv * env, jobject obj, jobject bitmap){
    NativeBitmap* nativeBitmap = convertBitmapToNative (env, bitmap);
    bool **image = convertToBoolmage(nativeBitmap);
    //vector<Train> train = createKnowledge(path_knowledge);

    vector<Train> trains;
    trains.push_back(Train('0',"RRRR"));
    trains.push_back(Train('1',"RLRLLLLRRLLLRRRLLLLRLLRRLRLLLRLLR"));

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
            string turns = generate_turn(sskode.str());
            vector<char> predictions = predict(turns, trains);

            DetectedChar detectedChar;
            detectedChar.start = border_info.start_point.x;
            detectedChar.value = predictions[0]; //guessChain(sskode.str(), knowledge);

            sskode << endl;
            detectedChar.chain = sskode.str();
            interpretation.push_back(detectedChar);

            //LOGD("%d, %d CHAIN CODE: %s", border_info.start_point.x, border_info.start_point.y, sskode.str().c_str());
            //LOGD("%s", turns.c_str());

        }
        //ss << guessChain(sskode.str());


        //sskode << endl;
        //fputs(sskode.str().c_str(), file);

    }
    //fclose(file);

    stringstream ss;

    //FILE* file = fopen("/sdcard/textTest.txt","w+");

    while(interpretation.size() > 0){
        int minValue = interpretation[0].start;
        int currentMin = 0;
        for (int i = 1; i < interpretation.size(); i++){
            if (interpretation[i].start < minValue){
                minValue = interpretation[i].start;
                currentMin = i;
            }
        }
        //fputs(interpretation[currentMin].chain.c_str(), file);
        ss << interpretation[currentMin].value;
        interpretation.erase(interpretation.begin() + currentMin);

    }
    //fclose(file);

    // [1] adalah ekspresi input, [2] adalah hasil perhitungan*/
    if (ss.str() == "0"){
        ss.str("HONDA");
    }else if (ss.str() == "1"){
        ss.str("TOYOTA");
    }
    std::string tes[] = { ss.str().c_str(), "44" };
    //std::string tes[] = { ss.str().c_str(), "44" };
    jobjectArray hasil2 = createJavaArray(env, 2, tes);

    return hasil2;
}