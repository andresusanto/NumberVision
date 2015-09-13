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

#define E  0
#define SE 1
#define S  2
#define SW 3
#define W  4
#define NW 5
#define N  6
#define NE 7

#define  LOG_TAG    "NUMBER VISION"
#define  LOGD(...)  __android_log_print(ANDROID_LOG_DEBUG,LOG_TAG,__VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)
#define NOT_SPECIFIED_COLOR -1

// supaya bisa dipanggil sama java
extern "C"
{
    JNIEXPORT jobjectArray JNICALL Java_com_ganesus_numbervision_MainActivity_detectAll (JNIEnv * env, jobject obj, jobject bitmap);
}


using namespace std;

/////////////////////////////////////////////////////////////////////////////////////
// helper class and functions
/////////////////////////////////////////////////////////////////////////////////////


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

struct PathElement {
    short direction;
    int ratio;

    PathElement(){
        this->direction = -1;
        this->ratio = 0;
    }

    PathElement(int direction,int ratio){
        this->direction = direction;
        this->ratio = ratio;
    }
};

struct Number
{
    int label;
    PathElement* path;
    int npath;

    Number(){}
    Number(int label,int npath,PathElement* path){
        this->npath = npath;
        this->label = label;
        this->path = (PathElement*) malloc(npath*sizeof(PathElement));
        for(int i = 0; i < npath; i++){
            this->path[i] = path[i];
        }

    }

    ~Number(){
        //free(path);
    }

    void print_path(){
        stringstream ss;
        LOGD("Jumlah = %d", this->npath);
        for(int i = 0; i < this->npath; i++){
            ss << "PathElement(" << path[i].direction << "," << path[i].direction << "), ";
            ss.seekg(0, ios::end);
            int size = ss.tellg();

            if (size == 900){
                LOGD("%s", ss.str().c_str());
                ss.clear();//clear any bits set
                ss.str(std::string());
            }
        }
        LOGD("%s", ss.str().c_str());
        //printf("\n");
    }

    int get_length(){
        int sum = 0;
        for(int i = 0; i < npath; i++){
            sum += path[i].ratio;
        }
        return sum;
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


Number chain_code_to_number(int label,short chain_code[],int code_length){

    int last_chain = -1;
    int counter = 0;
    queue<PathElement> que;

    for(int i = 0; i < code_length; i++){
        if (last_chain != chain_code[i]){
            if (counter > 0){
                que.push(PathElement(last_chain,counter));
            }
            last_chain = chain_code[i];
            counter = 1;
        } else {
            counter++;
        }
    }
    if (counter > 0){
        que.push(PathElement(last_chain,counter));
    }
    PathElement* path = new PathElement[que.size()];
    int i = 0;
    while(!que.empty()){
        path[i] = que.front(); que.pop();
        i++;
    }
    return Number(label,i,path);
}

double get_point(int expected_direction,int actual_direction){
    int i = expected_direction;
    int cwcounter = 0;
    while (i != actual_direction){
        i++; cwcounter++;
        if (i > 7) i %= 8;
    }
    i = expected_direction;
    int ccwcounter = 0;
    while (i != actual_direction){
        i--; ccwcounter++;
        if (i < 0) i += 8;
    }
    int diff = min(cwcounter,ccwcounter);
    if (diff == 0){
        return 1;
    } else if (diff == 1){
        return 0.5;
    } else if (diff == 2){
        return -1;
    } else {
        return -5;
    }
}


double get_match_result(short chain_code[],int code_length,Number number){
#ifdef DEBUG
    printf("-------\n");
    printf("match with number = %d\n",number.label);
#endif
    int n_length = number.get_length();
    double scale_ratio = (code_length * 1.0)/n_length;
    int last_path_length = 0;

    double point_sum = 0;
    for(int p = 0; p < number.npath; p++){
        double points = 0;
        int start = (int) floor(last_path_length * scale_ratio);
        int offset = (int) ceil(start + number.path[p].ratio*scale_ratio);
        int end = min(offset,code_length);
#ifdef DEBUG
        printf("match with %d, ratio %d\n",number.path[p].direction,number.path[p].ratio);
#endif
        for(int i = start; i < end; i++){
            points += get_point(number.path[p].direction,chain_code[i]);
            //printf("%d",chain_code[i]);
        }
#ifdef DEBUG
        printf("\n");
        printf("points = %.3lf\n",points);
        printf("\n");
#endif
        point_sum += points;
        last_path_length += number.path[p].ratio;
    }
    double ret = point_sum / n_length;
#ifdef DEBUG
    printf("%.3lf\n",ret);
    printf("------\n");
#endif
    return ret;

}

int match_chain_code(short chain_code[],int code_length,vector<Number> numbers,int nnumbers){
    double maks = -1;
    int label = -1;
    for(int i = 0; i < nnumbers; ++i){
        //double res = 0;
        double res = get_match_result(chain_code,code_length,numbers[i]);
        printf("%d %lf\n",numbers[i].label,res);
        if (maks < res){
            label = numbers[i].label;
            maks = res;
        }
    }
    return label;
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


////////////////////////////////////////////////////////////////////////////////////////

// PATTERN

////////////////////////////////////////////////////////////////////////////////////////


PathElement path_satu[] = {PathElement(0,0), PathElement(2,2), PathElement(1,1), PathElement(0,0), PathElement(2,2), PathElement(4,4), PathElement(6,6), PathElement(0,0), PathElement(7,7), PathElement(6,6), PathElement(5,5), PathElement(4,4), PathElement(3,3), PathElement(2,2), PathElement(4,4), PathElement(3,3), PathElement(2,2), PathElement(4,4), PathElement(3,3), PathElement(2,2), PathElement(4,4), PathElement(6,6), PathElement(5,5), PathElement(4,4), PathElement(6,6), PathElement(0,0), PathElement(7,7), PathElement(6,6), PathElement(0,0), PathElement(7,7), PathElement(6,6), PathElement(0,0), PathElement(7,7), PathElement(6,6), PathElement(0,0), PathElement(7,7), PathElement(6,6), PathElement(0,0), PathElement(7,7), PathElement(6,6), PathElement(0,0), PathElement(7,7), PathElement(6,6), PathElement(0,0), PathElement(7,7), PathElement(6,6), PathElement(0,0), PathElement(7,7), PathElement(6,6) };

PathElement path_dua[] = { PathElement(0,0), PathElement(2,2), PathElement(1,1), PathElement(0,0), PathElement(2,2), PathElement(1,1), PathElement(0,0), PathElement(2,2), PathElement(1,1), PathElement(0,0), PathElement(2,2), PathElement(1,1), PathElement(0,0), PathElement(2,2), PathElement(1,1), PathElement(0,0), PathElement(2,2), PathElement(1,1), PathElement(0,0), PathElement(2,2), PathElement(4,4), PathElement(3,3), PathElement(2,2), PathElement(4,4), PathElement(3,3), PathElement(2,2), PathElement(4,4), PathElement(3,3), PathElement(2,2), PathElement(4,4), PathElement(3,3), PathElement(2,2), PathElement(4,4), PathElement(3,3), PathElement(2,2), PathElement(4,4), PathElement(3,3), PathElement(2,2), PathElement(4,4), PathElement(3,3), PathElement(2,2), PathElement(4,4), PathElement(3,3), PathElement(2,2), PathElement(4,4), PathElement(3,3), PathElement(2,2), PathElement(4,4), PathElement(3,3), PathElement(2,2),
                           PathElement(4,4), PathElement(3,3), PathElement(2,2), PathElement(4,4), PathElement(3,3), PathElement(2,2), PathElement(4,4), PathElement(3,3), PathElement(2,2), PathElement(4,4), PathElement(3,3), PathElement(2,2), PathElement(4,4), PathElement(3,3), PathElement(2,2), PathElement(4,4), PathElement(3,3), PathElement(2,2), PathElement(4,4), PathElement(3,3), PathElement(1,1), PathElement(0,0), PathElement(2,2), PathElement(1,1), PathElement(0,0), PathElement(2,2), PathElement(4,4), PathElement(3,3), PathElement(2,2), PathElement(4,4), PathElement(6,6), PathElement(0,0), PathElement(7,7), PathElement(6,6), PathElement(0,0), PathElement(7,7), PathElement(6,6), PathElement(0,0), PathElement(7,7), PathElement(6,6), PathElement(0,0), PathElement(7,7), PathElement(6,6), PathElement(0,0), PathElement(7,7), PathElement(6,6), PathElement(0,0), PathElement(7,7), PathElement(6,6), PathElement(0,0),
                           PathElement(7,7), PathElement(6,6), PathElement(0,0), PathElement(7,7), PathElement(6,6), PathElement(0,0), PathElement(7,7), PathElement(6,6), PathElement(0,0), PathElement(7,7), PathElement(6,6), PathElement(0,0), PathElement(7,7), PathElement(6,6), PathElement(0,0), PathElement(7,7), PathElement(6,6), PathElement(0,0), PathElement(7,7), PathElement(6,6), PathElement(0,0), PathElement(7,7), PathElement(6,6), PathElement(0,0), PathElement(7,7), PathElement(6,6), PathElement(0,0), PathElement(7,7), PathElement(6,6), PathElement(0,0), PathElement(7,7), PathElement(6,6), PathElement(5,5), PathElement(4,4), PathElement(6,6), PathElement(5,5), PathElement(4,4), PathElement(6,6), PathElement(5,5), PathElement(4,4), PathElement(6,6), PathElement(5,5), PathElement(4,4), PathElement(3,3), PathElement(2,2), PathElement(4,4), PathElement(3,3), PathElement(2,2), PathElement(4,4), PathElement(3,3),
                           PathElement(2,2), PathElement(4,4), PathElement(6,6), PathElement(5,5), PathElement(4,4), PathElement(6,6), PathElement(0,0), PathElement(7,7), PathElement(6,6), PathElement(0,0), PathElement(7,7), PathElement(6,6), PathElement(0,0), PathElement(7,7), PathElement(6,6), PathElement(0,0), PathElement(7,7), PathElement(6,6)};


PathElement path_tiga[] = {PathElement(0,0), PathElement(2,2), PathElement(1,1), PathElement(0,0), PathElement(2,2), PathElement(1,1), PathElement(0,0), PathElement(2,2), PathElement(1,1), PathElement(0,0), PathElement(2,2), PathElement(1,1), PathElement(0,0), PathElement(2,2), PathElement(1,1), PathElement(0,0), PathElement(2,2), PathElement(4,4), PathElement(3,3), PathElement(2,2), PathElement(4,4), PathElement(3,3), PathElement(2,2), PathElement(4,4), PathElement(3,3), PathElement(2,2), PathElement(4,4), PathElement(3,3), PathElement(2,2), PathElement(4,4), PathElement(3,3), PathElement(1,1), PathElement(0,0), PathElement(2,2), PathElement(1,1), PathElement(0,0), PathElement(2,2), PathElement(1,1), PathElement(0,0), PathElement(2,2), PathElement(1,1), PathElement(0,0), PathElement(2,2), PathElement(1,1), PathElement(0,0), PathElement(2,2), PathElement(1,1), PathElement(0,0), PathElement(2,2), PathElement(4,4),
                           PathElement(3,3), PathElement(2,2), PathElement(4,4), PathElement(3,3), PathElement(2,2), PathElement(4,4), PathElement(3,3), PathElement(2,2), PathElement(4,4), PathElement(3,3), PathElement(2,2), PathElement(4,4), PathElement(3,3), PathElement(2,2), PathElement(4,4), PathElement(3,3), PathElement(2,2), PathElement(4,4), PathElement(6,6), PathElement(5,5), PathElement(4,4), PathElement(6,6), PathElement(5,5), PathElement(4,4), PathElement(6,6), PathElement(0,0), PathElement(2,2), PathElement(1,1), PathElement(0,0), PathElement(2,2), PathElement(1,1), PathElement(0,0), PathElement(2,2), PathElement(1,1), PathElement(0,0), PathElement(7,7), PathElement(6,6), PathElement(0,0), PathElement(7,7), PathElement(6,6), PathElement(0,0), PathElement(7,7), PathElement(6,6), PathElement(0,0), PathElement(7,7), PathElement(6,6), PathElement(0,0), PathElement(7,7), PathElement(5,5), PathElement(4,4),
                           PathElement(6,6), PathElement(5,5), PathElement(4,4), PathElement(6,6), PathElement(5,5), PathElement(4,4), PathElement(6,6), PathElement(5,5), PathElement(4,4), PathElement(6,6), PathElement(5,5), PathElement(4,4), PathElement(6,6), PathElement(5,5), PathElement(4,4), PathElement(6,6), PathElement(0,0), PathElement(7,7), PathElement(6,6), PathElement(0,0), PathElement(7,7), PathElement(6,6), PathElement(0,0), PathElement(7,7), PathElement(6,6), PathElement(0,0), PathElement(7,7), PathElement(6,6), PathElement(0,0), PathElement(7,7), PathElement(6,6), PathElement(5,5), PathElement(4,4), PathElement(6,6), PathElement(5,5), PathElement(4,4), PathElement(6,6), PathElement(5,5), PathElement(4,4), PathElement(6,6), PathElement(5,5), PathElement(4,4), PathElement(3,3), PathElement(2,2), PathElement(4,4), PathElement(3,3), PathElement(2,2), PathElement(4,4), PathElement(3,3), PathElement(2,2),
                           PathElement(4,4), PathElement(3,3), PathElement(2,2), PathElement(4,4), PathElement(6,6), PathElement(0,0), PathElement(7,7), PathElement(6,6), PathElement(0,0), PathElement(7,7), PathElement(6,6), PathElement(0,0), PathElement(7,7), PathElement(6,6)
};

PathElement path_empat[] = {PathElement(0,0), PathElement(2,2), PathElement(1,1), PathElement(0,0), PathElement(2,2), PathElement(4,4), PathElement(3,3), PathElement(2,2), PathElement(4,4), PathElement(6,6), PathElement(5,5), PathElement(4,4), PathElement(6,6), PathElement(5,5), PathElement(4,4), PathElement(6,6), PathElement(0,0), PathElement(7,7), PathElement(6,6), PathElement(0,0), PathElement(7,7), PathElement(6,6), PathElement(0,0), PathElement(7,7), PathElement(6,6), PathElement(0,0), PathElement(7,7), PathElement(6,6), PathElement(0,0), PathElement(7,7), PathElement(6,6), PathElement(0,0), PathElement(7,7), PathElement(6,6), PathElement(0,0), PathElement(7,7), PathElement(6,6), PathElement(0,0), PathElement(7,7), PathElement(6,6), PathElement(0,0), PathElement(7,7), PathElement(6,6), PathElement(0,0), PathElement(7,7), PathElement(6,6), PathElement(0,0), PathElement(7,7), PathElement(6,6), PathElement(0,0),
                            PathElement(7,7), PathElement(6,6), PathElement(0,0), PathElement(7,7), PathElement(6,6), PathElement(0,0), PathElement(7,7), PathElement(6,6), PathElement(0,0), PathElement(7,7), PathElement(6,6), PathElement(0,0), PathElement(7,7), PathElement(6,6)};

PathElement path_lima[] = {PathElement(0,0), PathElement(2,2), PathElement(1,1), PathElement(0,0), PathElement(2,2), PathElement(4,4), PathElement(3,3), PathElement(2,2), PathElement(4,4), PathElement(3,3), PathElement(2,2), PathElement(1,1), PathElement(0,0), PathElement(2,2), PathElement(1,1), PathElement(0,0), PathElement(2,2), PathElement(1,1), PathElement(0,0), PathElement(2,2), PathElement(1,1), PathElement(0,0), PathElement(2,2), PathElement(1,1), PathElement(0,0), PathElement(2,2), PathElement(1,1), PathElement(0,0), PathElement(2,2), PathElement(1,1), PathElement(0,0), PathElement(2,2), PathElement(4,4), PathElement(3,3), PathElement(2,2), PathElement(4,4), PathElement(3,3), PathElement(2,2), PathElement(4,4), PathElement(3,3), PathElement(2,2), PathElement(4,4), PathElement(3,3), PathElement(2,2), PathElement(4,4), PathElement(3,3), PathElement(2,2), PathElement(4,4), PathElement(3,3), PathElement(2,2),
                           PathElement(4,4), PathElement(3,3), PathElement(2,2), PathElement(4,4), PathElement(6,6), PathElement(5,5), PathElement(4,4), PathElement(6,6), PathElement(5,5), PathElement(4,4), PathElement(6,6), PathElement(0,0), PathElement(2,2), PathElement(1,1), PathElement(0,0), PathElement(2,2), PathElement(1,1), PathElement(0,0), PathElement(2,2), PathElement(1,1), PathElement(0,0), PathElement(7,7), PathElement(6,6), PathElement(0,0), PathElement(7,7), PathElement(6,6), PathElement(0,0), PathElement(7,7), PathElement(6,6), PathElement(0,0), PathElement(7,7), PathElement(6,6), PathElement(0,0), PathElement(7,7), PathElement(6,6), PathElement(5,5), PathElement(4,4), PathElement(6,6), PathElement(5,5), PathElement(4,4), PathElement(6,6), PathElement(5,5), PathElement(4,4), PathElement(6,6), PathElement(5,5), PathElement(4,4), PathElement(6,6), PathElement(5,5), PathElement(4,4), PathElement(6,6),
                           PathElement(0,0), PathElement(7,7), PathElement(6,6)};

PathElement path_enam[] = {PathElement(0,0), PathElement(2,2), PathElement(1,1), PathElement(0,0), PathElement(2,2), PathElement(4,4), PathElement(6,6), PathElement(5,5), PathElement(4,4), PathElement(3,3), PathElement(2,2), PathElement(4,4), PathElement(3,3), PathElement(2,2), PathElement(4,4), PathElement(3,3), PathElement(2,2), PathElement(4,4), PathElement(3,3), PathElement(2,2), PathElement(4,4), PathElement(3,3), PathElement(2,2), PathElement(4,4), PathElement(3,3), PathElement(2,2), PathElement(4,4), PathElement(3,3), PathElement(2,2), PathElement(1,1), PathElement(0,0), PathElement(7,7), PathElement(6,6), PathElement(0,0), PathElement(2,2), PathElement(1,1), PathElement(0,0), PathElement(2,2), PathElement(1,1), PathElement(0,0), PathElement(2,2), PathElement(1,1), PathElement(0,0), PathElement(2,2), PathElement(1,1), PathElement(0,0), PathElement(2,2), PathElement(1,1), PathElement(0,0), PathElement(2,2),
                           PathElement(4,4), PathElement(3,3), PathElement(2,2), PathElement(4,4), PathElement(3,3), PathElement(2,2), PathElement(4,4), PathElement(3,3), PathElement(2,2), PathElement(4,4), PathElement(3,3), PathElement(2,2), PathElement(4,4), PathElement(3,3), PathElement(2,2), PathElement(4,4), PathElement(3,3), PathElement(2,2), PathElement(4,4), PathElement(3,3), PathElement(2,2), PathElement(4,4), PathElement(6,6), PathElement(5,5), PathElement(4,4), PathElement(6,6), PathElement(5,5), PathElement(4,4), PathElement(6,6), PathElement(5,5), PathElement(4,4), PathElement(6,6), PathElement(5,5), PathElement(4,4), PathElement(6,6), PathElement(5,5), PathElement(4,4), PathElement(6,6), PathElement(5,5), PathElement(4,4), PathElement(6,6), PathElement(0,0), PathElement(7,7), PathElement(6,6), PathElement(0,0), PathElement(7,7), PathElement(6,6), PathElement(0,0), PathElement(7,7), PathElement(6,6),
                           PathElement(0,0), PathElement(7,7), PathElement(6,6), PathElement(0,0), PathElement(7,7), PathElement(6,6), PathElement(0,0), PathElement(7,7), PathElement(6,6), PathElement(0,0), PathElement(7,7), PathElement(6,6), PathElement(0,0), PathElement(7,7), PathElement(6,6), PathElement(0,0), PathElement(7,7), PathElement(6,6)};

PathElement path_tujuh[] = {PathElement(0,0), PathElement(2,2), PathElement(4,4), PathElement(3,3), PathElement(2,2), PathElement(4,4), PathElement(3,3), PathElement(2,2), PathElement(4,4), PathElement(3,3), PathElement(2,2), PathElement(4,4), PathElement(3,3), PathElement(2,2), PathElement(4,4), PathElement(3,3), PathElement(2,2), PathElement(4,4), PathElement(3,3), PathElement(2,2), PathElement(4,4), PathElement(3,3), PathElement(2,2), PathElement(4,4), PathElement(3,3), PathElement(2,2), PathElement(4,4), PathElement(3,3), PathElement(2,2), PathElement(4,4), PathElement(3,3), PathElement(2,2), PathElement(4,4), PathElement(3,3), PathElement(2,2), PathElement(4,4), PathElement(3,3), PathElement(2,2), PathElement(4,4), PathElement(3,3), PathElement(2,2), PathElement(4,4), PathElement(3,3), PathElement(2,2), PathElement(4,4), PathElement(3,3), PathElement(2,2), PathElement(4,4), PathElement(3,3), PathElement(2,2),
                            PathElement(4,4), PathElement(6,6), PathElement(0,0), PathElement(7,7), PathElement(6,6), PathElement(0,0), PathElement(7,7), PathElement(6,6), PathElement(0,0), PathElement(7,7), PathElement(6,6), PathElement(0,0), PathElement(7,7), PathElement(6,6), PathElement(0,0), PathElement(7,7), PathElement(6,6), PathElement(0,0), PathElement(7,7), PathElement(6,6), PathElement(0,0), PathElement(7,7), PathElement(6,6), PathElement(0,0), PathElement(7,7), PathElement(6,6), PathElement(0,0), PathElement(7,7), PathElement(6,6), PathElement(0,0), PathElement(7,7), PathElement(6,6), PathElement(0,0), PathElement(7,7), PathElement(6,6), PathElement(0,0), PathElement(7,7), PathElement(6,6), PathElement(0,0), PathElement(7,7), PathElement(6,6), PathElement(0,0), PathElement(7,7), PathElement(6,6), PathElement(0,0), PathElement(7,7), PathElement(5,5), PathElement(4,4), PathElement(6,6), PathElement(5,5),
                            PathElement(4,4), PathElement(6,6), PathElement(0,0), PathElement(7,7), PathElement(6,6)};

PathElement path_delapan[] = {PathElement(0,0), PathElement(2,2), PathElement(1,1), PathElement(0,0), PathElement(2,2), PathElement(1,1), PathElement(0,0), PathElement(2,2), PathElement(1,1), PathElement(0,0), PathElement(2,2), PathElement(1,1), PathElement(0,0), PathElement(2,2), PathElement(1,1), PathElement(0,0), PathElement(2,2), PathElement(4,4), PathElement(3,3), PathElement(2,2), PathElement(4,4), PathElement(3,3), PathElement(2,2), PathElement(4,4), PathElement(3,3), PathElement(2,2), PathElement(4,4), PathElement(3,3), PathElement(2,2), PathElement(4,4), PathElement(3,3), PathElement(2,2), PathElement(4,4), PathElement(3,3), PathElement(2,2), PathElement(1,1), PathElement(0,0), PathElement(2,2), PathElement(1,1), PathElement(0,0), PathElement(2,2), PathElement(1,1), PathElement(0,0), PathElement(2,2), PathElement(1,1), PathElement(0,0), PathElement(2,2), PathElement(1,1), PathElement(0,0), PathElement(2,2),
                              PathElement(1,1), PathElement(0,0), PathElement(2,2), PathElement(4,4), PathElement(3,3), PathElement(2,2), PathElement(4,4), PathElement(3,3), PathElement(2,2), PathElement(4,4), PathElement(3,3), PathElement(2,2), PathElement(4,4), PathElement(3,3), PathElement(2,2), PathElement(4,4), PathElement(3,3), PathElement(2,2), PathElement(4,4), PathElement(3,3), PathElement(2,2), PathElement(4,4), PathElement(6,6), PathElement(5,5), PathElement(4,4), PathElement(6,6), PathElement(5,5), PathElement(4,4), PathElement(6,6), PathElement(5,5), PathElement(4,4), PathElement(6,6), PathElement(5,5), PathElement(4,4), PathElement(6,6), PathElement(5,5), PathElement(4,4), PathElement(6,6), PathElement(0,0), PathElement(7,7), PathElement(6,6), PathElement(0,0), PathElement(7,7), PathElement(6,6), PathElement(0,0), PathElement(7,7), PathElement(6,6), PathElement(0,0), PathElement(7,7), PathElement(6,6),
                              PathElement(0,0), PathElement(7,7), PathElement(6,6), PathElement(0,0), PathElement(7,7), PathElement(6,6), PathElement(0,0), PathElement(7,7), PathElement(5,5), PathElement(4,4), PathElement(6,6), PathElement(5,5), PathElement(4,4), PathElement(6,6), PathElement(5,5), PathElement(4,4), PathElement(6,6), PathElement(5,5), PathElement(4,4), PathElement(6,6), PathElement(5,5), PathElement(4,4), PathElement(6,6), PathElement(5,5), PathElement(4,4), PathElement(6,6), PathElement(0,0), PathElement(7,7), PathElement(6,6), PathElement(0,0), PathElement(7,7), PathElement(6,6), PathElement(0,0), PathElement(7,7), PathElement(6,6), PathElement(0,0), PathElement(7,7), PathElement(6,6), PathElement(0,0), PathElement(7,7), PathElement(6,6), PathElement(0,0), PathElement(7,7), PathElement(6,6)};

PathElement path_sembilan[] = {PathElement(0,0), PathElement(2,2), PathElement(1,1), PathElement(0,0), PathElement(2,2), PathElement(1,1), PathElement(0,0), PathElement(2,2), PathElement(1,1), PathElement(0,0), PathElement(2,2), PathElement(1,1), PathElement(0,0), PathElement(2,2), PathElement(1,1), PathElement(0,0), PathElement(2,2), PathElement(1,1), PathElement(0,0), PathElement(2,2), PathElement(1,1), PathElement(0,0), PathElement(2,2), PathElement(1,1), PathElement(0,0), PathElement(2,2), PathElement(4,4), PathElement(3,3), PathElement(2,2), PathElement(4,4), PathElement(3,3), PathElement(2,2), PathElement(4,4), PathElement(3,3), PathElement(2,2), PathElement(4,4), PathElement(3,3), PathElement(2,2), PathElement(4,4), PathElement(3,3), PathElement(2,2), PathElement(4,4), PathElement(3,3), PathElement(2,2), PathElement(4,4), PathElement(3,3), PathElement(2,2), PathElement(4,4), PathElement(3,3), PathElement(2,2),
                               PathElement(4,4), PathElement(3,3), PathElement(2,2), PathElement(4,4), PathElement(3,3), PathElement(2,2), PathElement(4,4), PathElement(6,6), PathElement(5,5), PathElement(4,4), PathElement(6,6), PathElement(5,5), PathElement(4,4), PathElement(6,6), PathElement(0,0), PathElement(2,2), PathElement(1,1), PathElement(0,0), PathElement(7,7), PathElement(6,6), PathElement(0,0), PathElement(7,7), PathElement(6,6), PathElement(0,0), PathElement(7,7), PathElement(6,6), PathElement(0,0), PathElement(7,7), PathElement(6,6), PathElement(0,0), PathElement(7,7), PathElement(6,6), PathElement(0,0), PathElement(7,7), PathElement(6,6), PathElement(0,0), PathElement(7,7), PathElement(6,6), PathElement(0,0), PathElement(7,7), PathElement(5,5), PathElement(4,4), PathElement(3,3), PathElement(2,2), PathElement(4,4), PathElement(6,6), PathElement(5,5), PathElement(4,4), PathElement(6,6), PathElement(5,5),
                               PathElement(4,4), PathElement(6,6), PathElement(5,5), PathElement(4,4), PathElement(6,6), PathElement(5,5), PathElement(4,4), PathElement(6,6), PathElement(5,5), PathElement(4,4), PathElement(6,6), PathElement(0,0), PathElement(7,7), PathElement(6,6), PathElement(0,0), PathElement(7,7), PathElement(6,6), PathElement(0,0), PathElement(7,7), PathElement(6,6), PathElement(0,0), PathElement(7,7), PathElement(6,6), PathElement(0,0), PathElement(7,7), PathElement(6,6), PathElement(0,0), PathElement(7,7), PathElement(6,6), PathElement(0,0), PathElement(7,7), PathElement(6,6)};

PathElement path_nol[] = {PathElement(0,0), PathElement(2,2), PathElement(1,1), PathElement(0,0), PathElement(2,2), PathElement(1,1), PathElement(0,0), PathElement(2,2), PathElement(1,1), PathElement(0,0), PathElement(2,2), PathElement(1,1), PathElement(0,0), PathElement(2,2), PathElement(1,1), PathElement(0,0), PathElement(2,2), PathElement(1,1), PathElement(0,0), PathElement(2,2), PathElement(1,1), PathElement(0,0), PathElement(2,2), PathElement(1,1), PathElement(0,0), PathElement(2,2), PathElement(4,4), PathElement(3,3), PathElement(2,2), PathElement(4,4), PathElement(3,3), PathElement(2,2), PathElement(4,4), PathElement(3,3), PathElement(2,2), PathElement(4,4), PathElement(3,3), PathElement(2,2), PathElement(4,4), PathElement(3,3), PathElement(2,2), PathElement(4,4), PathElement(3,3), PathElement(2,2), PathElement(4,4), PathElement(3,3), PathElement(2,2), PathElement(4,4), PathElement(3,3), PathElement(2,2),
                          PathElement(4,4), PathElement(6,6), PathElement(5,5), PathElement(4,4), PathElement(6,6), PathElement(5,5), PathElement(4,4), PathElement(6,6), PathElement(5,5), PathElement(4,4), PathElement(6,6), PathElement(5,5), PathElement(4,4), PathElement(6,6), PathElement(5,5), PathElement(4,4), PathElement(6,6), PathElement(5,5), PathElement(4,4), PathElement(6,6), PathElement(5,5), PathElement(4,4), PathElement(6,6), PathElement(5,5), PathElement(4,4), PathElement(6,6), PathElement(0,0), PathElement(7,7), PathElement(6,6), PathElement(0,0), PathElement(7,7), PathElement(6,6), PathElement(0,0), PathElement(7,7), PathElement(6,6), PathElement(0,0), PathElement(7,7), PathElement(6,6), PathElement(0,0), PathElement(7,7), PathElement(6,6), PathElement(0,0), PathElement(7,7), PathElement(6,6), PathElement(0,0), PathElement(7,7), PathElement(6,6), PathElement(0,0), PathElement(7,7), PathElement(6,6)};

PathElement path_tambah[] = {PathElement(0,0), PathElement(2,2), PathElement(1,1), PathElement(0,0), PathElement(2,2), PathElement(4,4), PathElement(3,3), PathElement(2,2), PathElement(4,4), PathElement(6,6), PathElement(5,5), PathElement(4,4), PathElement(6,6), PathElement(5,5), PathElement(4,4), PathElement(6,6), PathElement(0,0), PathElement(7,7), PathElement(6,6), PathElement(0,0), PathElement(7,7), PathElement(6,6)};
PathElement path_kurang[] = {PathElement(0,0), PathElement(2,2), PathElement(4,4), PathElement(6,6), PathElement(5,5), PathElement(4,4), PathElement(6,6), PathElement(0,0), PathElement(7,7), PathElement(6,6)};

///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////


JNIEXPORT jobjectArray JNICALL Java_com_ganesus_numbervision_MainActivity_detectAll (JNIEnv * env, jobject obj, jobject bitmap){
    NativeBitmap* nativeBitmap = convertBitmapToNative (env, bitmap);
    bool **image;

    vector<Number> numbers;


    numbers.push_back( Number(1, 49,path_satu) );
    numbers.push_back( Number(2, 168,path_dua) );
    numbers.push_back( Number(3, 164,path_tiga) );
    numbers.push_back( Number(4, 64,path_empat) );
    numbers.push_back( Number(5, 103,path_lima) );
    numbers.push_back( Number(6, 118,path_enam) );
    numbers.push_back( Number(7, 105,path_tujuh) );
    numbers.push_back( Number(8, 144,path_delapan) );
    numbers.push_back( Number(9, 132,path_sembilan) );
    numbers.push_back( Number(0, 100,path_nol) );
    numbers.push_back( Number(11, 22,path_tambah) );
    numbers.push_back( Number(12, 10,path_kurang) );

    image = new bool*[nativeBitmap->bitmapInfo.height];
    for (int i=0;i<nativeBitmap->bitmapInfo.height;i++) {
        image[i] = new bool[nativeBitmap->bitmapInfo.width];
        for (int j=0;j<nativeBitmap->bitmapInfo.width;j++) {
            ARGB warna;
            convertIntToArgb(nativeBitmap->pixels[i * nativeBitmap->bitmapInfo.width + j], &warna);

            image[i][j] = !(warna.blue == 255 && warna.green == 255 && warna.red == 255);
        }
    }

    vector<BorderInfo> border_infos = get_border_infos(image,nativeBitmap->bitmapInfo.width,nativeBitmap->bitmapInfo.height);
    delete nativeBitmap;

    for (int i=0;i<border_infos.size();i++) {
        BorderInfo border_info = border_infos[i];

        int ukuran = border_info.chain_codes.size();
        stringstream ss;

        short* test = new short[ukuran];
        for (int j=0;j<border_info.chain_codes.size();j++) {
            test[j] = border_info.chain_codes[j];
            ss << border_info.chain_codes[j];
            //LOGD("%d \n", border_info.chain_codes[j]);
            //cout<<border_info.chain_codes[j]<<endl;
        }
        LOGD("KIRA KIRA = %d \n", match_chain_code(test,ukuran,numbers, numbers.size()));

    }


    // [1] adalah ekspresi input, [2] adalah hasil perhitungan
    std::string tes[] = { "12+32", "44" };
    jobjectArray hasil = createJavaArray(env, 2, tes);

    return hasil;
}