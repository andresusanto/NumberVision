#include "turncode.h"

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

bool is_u_turn_cw(double s,double d){
    double diff = fabs(s - d);
    if (diff > 180.0) diff = 360.0 - diff;
    if (diff < 150.0) return false;

    s = s + diff;
    if (s > 360.0) s -= 360.0;
    return fabs(s - d) <= 0.0001;
}

bool is_u_turn_ccw(double s,double d){
    double diff = fabs(s - d);
    if (diff > 180.0) diff = 360.0 - diff;
    if (diff < 150.0) return false;

    d = d + diff;
    if (d > 360.0) d -= 360.0;
    return fabs(s - d) <= 0.0001;
}

string generate_turn(const string &code){
    LOGD("generate turn");
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
        } else if (is_u_turn_cw(dl,dr)){
            ret += RIGHT; ret += RIGHT;
            i += LENGTH_TAKE /2;
        } else if (is_u_turn_ccw(dl,dr)) {
            ret += LEFT; ret += LEFT;
            i += LENGTH_TAKE /2;
        } else {
            i++;
        }
    }
    LOGD("generate turn end");
    return ret;
}


int edit_distance(const string& a,const string& b){
    LOGD("edit distance start");
    int dp[1000][1000];

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
    LOGD("edit distance end");
    return dp[a.size()][b.size()];
}

vector<char> predict(const string& chain_code,const vector<Train>& trains){
    LOGD("predict start");
    vector<char> probabilities;
    probabilities.clear();

    int cost = edit_distance(chain_code,trains[0].path);
    LOGD("predict prob before");
    probabilities.push_back(trains[0].label);
    LOGD("predict prob after");
    for(int i = 1; i < (int) trains.size(); i++){
        LOGD("%d",i);
        int cnow = edit_distance(chain_code,trains[i].path);
        if (cnow < cost){
            cost = cnow;
            probabilities.clear();
            probabilities.push_back(trains[i].label);
        } else if (cnow == cost){
            probabilities.push_back(trains[i].label);
        }
    }
    LOGD("predict end");
    return probabilities;
}
