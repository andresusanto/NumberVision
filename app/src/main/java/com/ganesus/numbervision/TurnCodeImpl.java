package com.ganesus.numbervision;

public class TurnCodeImpl implements TurnCode{
    static final int LENGTH_TAKE = 20;
    static final char E = '0';
    static final char S = '2';
    static final char W = '4';
    static final char N = '6';
    static final char RIGHT = 'R';
    static final char LEFT = 'L';
    //derajat minimal agar dia dikatakan belok
    static final double MIN_D = 50.0;
    //derajat maximal agar dia dikatakan belok
    static final double MAX_D = 90.0;

    @Override
    public String generateTurn(String code){
        StringBuilder ret = new StringBuilder();
        for(int i = 0; i + LENGTH_TAKE < code.length(); ){
            String sleft = code.substring(i, i + LENGTH_TAKE / 2);
            String sright = code.substring(i + (LENGTH_TAKE / 2), i + i + (LENGTH_TAKE/2)+ LENGTH_TAKE/2);
            double dl = degreeByVote(sleft);
            double dr = degreeByVote(sright);
            double diff = Math.abs(dl - dr);
            if (diff > 180.0) diff = 360. - diff;
            if (isLeftTurn(dl, dr)){
                ret.append(LEFT);
                i += LENGTH_TAKE/2;
                //System.out.printf("got left turn at %d, %s %s, deg = %.3lf %.3lf %.3lf\n", i, sleft, sright, dl, dr, diff);
            } else if (isRightTurn(dl, dr)){
                ret.append(RIGHT);
                i += LENGTH_TAKE/2;
                //System.out.printf("got right turn at %d, %s %s , deg = %.3lf %.3lf %.3lf\n", i, sleft, sright, dl, dr, diff);
            } else if (isUTurnCw(dl, dr)){
                ret.append(RIGHT); ret.append(RIGHT);
                i += LENGTH_TAKE /2;
            } else if (isUTurnCcw(dl, dr)) {
                ret.append(LEFT); ret.append(LEFT);
                i += LENGTH_TAKE /2;
            } else {
                i++;
            }
        }
        return ret.toString();
    }

    double degreeByVote(String test){
        double dx[] = { 1, 1, 0,-1,-1,-1, 0, 1};
        double dy[] = { 0,-1,-1,-1, 0, 1, 1, 1};

        double x = 0;
        double y = 0;
        for(int i = 0; i < (int) test.length(); i++){
            int _dir = (int) (test.charAt(i) - '0');
            x += dx[_dir];
            y += dy[_dir];
            //printf("%c %.3lf\n",test[i],deg);
        }
        double temp = Math.atan2(-y, x) * 180 / 3.14159265;
        temp += 360.;
        if (temp > 360.0) temp -= 360.0;
        return temp;
    }


    boolean isRightTurn(double s, double d){
        double diff = Math.abs(s - d);
        if (diff > 180.0) diff = 360.0 - diff;
        if (diff < MIN_D || diff > MAX_D) return false;

        s = s + diff;
        if (s > 360.0) s -= 360.0;
        return Math.abs(s - d) <= 0.0001;
    }

    boolean isLeftTurn(double s, double d){
        double diff = Math.abs(s - d);
        if (diff > 180.0) diff = 360.0 - diff;
        if (diff < MIN_D || diff > MAX_D) return false;

        d = d + diff;
        if (d > 360.0) d -= 360.0;
        return Math.abs(s - d) <= 0.0001;
    }

    boolean isUTurnCw(double s, double d){
        double diff = Math.abs(s - d);
        if (diff > 180.0) diff = 360.0 - diff;
        if (diff < 150.0) return false;

        s = s + diff;
        if (s > 360.0) s -= 360.0;
        return Math.abs(s - d) <= 0.0001;
    }

    boolean isUTurnCcw(double s, double d){
        double diff = Math.abs(s - d);
        if (diff > 180.0) diff = 360.0 - diff;
        if (diff < 150.0) return false;

        d = d + diff;
        if (d > 360.0) d -= 360.0;
        return Math.abs(s - d) <= 0.0001;
    }

}
