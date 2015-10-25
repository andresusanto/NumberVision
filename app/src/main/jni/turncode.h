#include <jni.h>
#include <android/log.h>
#include <string>
#include <queue>

#define  LOG_TAG    "NUMBER VISION"
#define  LOGD(...)  __android_log_print(ANDROID_LOG_DEBUG,LOG_TAG,__VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)

#define E '0'
#define S '2'
#define W '4'
#define N '6'
#define RIGHT 'R'
#define LEFT 'L'

//subchain code data yang diambil
#define LENGTH_TAKE 20
//derajat minimal agar dia dikatakan belok
#define MIN_D 50.0
//derajat maximal agar dia dikatakan belok
#define MAX_D 90.0
//maximal banyak kode belok
#define MAX_TURN_CODE 50

using namespace std;

struct Train {
    char label;
    string path;
    Train(char label,string path){
        this->label = label;
        this->path = path;
    }
};


double degree_by_vote(const string &test);
bool is_right_turn(double s,double d);
bool is_left_turn(double s,double d);
bool is_u_turn_cw(double s,double d);
bool is_u_turn_ccw(double s,double d);
string generate_turn(const string &code);
int edit_distance(const string& a,const string& b);
vector<char> predict(const string& chain_code,const vector<Train>& trains);