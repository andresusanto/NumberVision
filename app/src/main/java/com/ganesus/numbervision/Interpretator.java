package com.ganesus.numbervision;

import android.content.res.Resources;
import android.util.Log;

import java.io.BufferedReader;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.util.LinkedList;
import java.util.List;
import java.util.Scanner;

/**
 * Created by Andre on 10/13/2015.
 */
public class Interpretator {
    private static final int STABILIZATION_FACTOR = 4;
    private List<Knowledge> knowledge;

    private static class DetectedChar{
        public char value;
        public String chain;
        public int start;
    }

    private static class Knowledge{
        public char meaning;
        public String data;

        Knowledge(char _mean, String _data){
            meaning = _mean;
            data = _data;
        }
    }

    private static class ECode{
        public char direction;
        public int value;

        ECode(char _dir, int _val){
            direction = _dir;
            value = _val;
        }
    }

    private List<ECode> stabileData(String original){
        List<ECode> training = new LinkedList<>();

        char currentDirection = original.charAt(0);
        int currentValue = 1;
        for (int i = 1 ; i < original.length(); i++){
            if (currentDirection == original.charAt(i))
                currentValue++;
            else{
                ECode kode = new ECode(currentDirection, currentValue);

                training.add(kode);

                currentValue = 1;
                currentDirection = original.charAt(i);
            }
        }

        ECode kode = new ECode(currentDirection, currentValue);
        training.add(kode);

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

    private float calculateChain (String strKnowledge, String strTest ){
        List<ECode> knowledge, test;

        List<ECode> data1 = stabileData(strKnowledge);
        List<ECode> data2 = stabileData(strTest);

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
            knowledgeChains += knowledge.get(i).value;

        for (int i = 0 ; i < testSize; i++)
            testChains += test.get(i).value;


        float currentScore = 0; int iteratorKnowledge = 0;

        for (int i = 0; i < testSize && iteratorKnowledge < knowledgeSize; i++){
            if (test.get(i).direction == knowledge.get(iteratorKnowledge).direction){
                currentScore += ((float)test.get(i).value / testChains + (float)knowledge.get(iteratorKnowledge).value / knowledgeChains) * 2.0f;
            }else{
                currentScore -= (float)knowledge.get(iteratorKnowledge).value / (knowledgeChains * 2.0f);
                i--;
            }
            iteratorKnowledge++;

            if (iteratorKnowledge == knowledgeSize){
                for (; i < testSize; i++){
                    currentScore -= (float)test.get(i).value / testChains;
                }
            }
        }


        return currentScore;
    }

    private void loadKnowledge(Resources r, int resourceID){
        InputStream is = r.openRawResource(resourceID);
        Scanner scanner = new Scanner(is);

        knowledge = new LinkedList<>();

        while (scanner.hasNext()){
            char tmpChar = scanner.next().charAt(0);
            String tmpString = scanner.next();

            Knowledge tmpKnowledge = new Knowledge(tmpChar, tmpString);
            knowledge.add(tmpKnowledge);
        }
    }

    public char guessChain(String chainCode){


        char currentChar = knowledge.get(0).meaning;
        float currentMax = calculateChain(knowledge.get(0).data, chainCode);
        for (int i = 1 ; i < knowledge.size(); i++){
            float rate = calculateChain(knowledge.get(i).data, chainCode);
            if (rate > currentMax){
                currentMax = rate;
                currentChar = knowledge.get(i).meaning;
            }
        }

        return currentChar;
    }

    Interpretator(Resources r, int resourceID){
        loadKnowledge(r, resourceID);
    }
}
