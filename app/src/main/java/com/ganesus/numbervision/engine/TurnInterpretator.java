package com.ganesus.numbervision.engine;

import java.util.List;

/**
 * Created by Andre on 10/25/2015.
 */
public class TurnInterpretator {
    public String tc;
    public char it;

    public TurnInterpretator(String tc,char it){
        this.tc = tc;
        this.it =  it;
    }

    private static int min(int a, int b){
        if (a < b) return a;
        return b;
    }

    public static int editDistance(String a,String b){
        int dpsz = Math.max(a.length(),b.length()) + 10;
        int [][] dp = new int[dpsz][dpsz];

        for(int ia = 1; ia <= (int) a.length(); ia++){
            dp[ia][0] = ia;
        }
        for(int ib = 1; ib <= (int) b.length(); ib++){
            dp[0][ib] = ib;
        }
        for(int ia = 1; ia <= (int) a.length(); ia++){
            for(int ib = 1; ib <= (int) b.length(); ib++){
                if (a.charAt(ia - 1) == b.charAt(ib- 1)){
                    dp[ia][ib] = dp[ia-1][ib-1];
                } else {
                    dp[ia][ib] = min(dp[ia-1][ib],min(dp[ia-1][ib-1],dp[ia][ib-1])) + 1;
                }
            }
        }
        return dp[a.length()][b.length()];
    }

    public static char intepret(String turnCode,List<TurnInterpretator> tclearns){
        int r = 0;
        int mr = editDistance(turnCode,tclearns.get(0).tc);
        for(int i = 1; i < tclearns.size(); i++){
            if (editDistance(turnCode,tclearns.get(i).tc) < mr){
                mr = editDistance(turnCode,tclearns.get(i).tc);
                r = i;
            }
        }
        return tclearns.get(r).it;
    }
}
