package com.ganesus.numbervision;

import java.util.ArrayList;
import java.util.List;

/**
 * Created by kevinyu on 10/25/15.
 */
public class ToNxN {
    private int N;
    private final double percentage=0.5;

    public ToNxN(int N){
        this.N=N;
    }

    private class PairPoint{
        /*
          bL 2 3
          4 5 6
          7 8 uR
         */
        private Point bottomLeft,upRight;
        public PairPoint(Point bL,Point uR){
            this.bottomLeft=bL;
            this.upRight=uR;
        }
        public Point getUpRight(){return upRight;}
        public Point getBotLeft(){return bottomLeft;}
    }

    private Point chcodeToPoint(Point start,char chcode){
        Point newPoint;
        switch(chcode) {
            case '0':
                newPoint=new Point(start.getX()+1,start.getY());
                break;
            case '1':
                newPoint= new Point(start.getX()+1,start.getY()+1);
                break;
            case '2':
                newPoint= new Point(start.getX(),start.getY()+1);
                break;
            case '3':
                newPoint= new Point(start.getX()-1,start.getY()+1);
                break;
            case '4':
                newPoint= new Point(start.getX()-1,start.getY());
                break;
            case '5':
                newPoint=new Point(start.getX()-1,start.getY()-1);
                break;
            case '6':
                newPoint=new Point(start.getX(),start.getY()-1);
                break;
            case '7':
                newPoint=new Point(start.getX()+1,start.getY()-1);
                break;
            default:
                newPoint=new Point(-1,-1);
                break;
        }
        return newPoint;
    }

    public boolean[][] singleToNxN(ChainCodeGenerator.BorderInfo border,boolean[][] boolImg){
        boolean[][] newBoolImg=new boolean[N+2][N+2];
        //Find reactangle prescribed the chaincode
        PairPoint pairP=findRectangle(border, boolImg);
        Point upRight=pairP.getUpRight();
        Point botLeft=pairP.getBotLeft();
        int width=upRight.getX()-botLeft.getX()+1;
        int smallWidth=width/N;
        int height=upRight.getY()-botLeft.getY()+1;
        int smallHeight=height/N;

        //Find majority of black in each cell
        Point iterate=botLeft;
        for(int row=1;row<=N;row++) {
            iterate.setY(botLeft.getY() + (row - 1) * smallHeight);
            for (int col = 1; col <= N; col++) {
                iterate.setX(botLeft.getX()+(col-1)*smallWidth);
                //count majority
                int imax=iterate.getX()+smallWidth-1;
                int jmax=iterate.getY()+smallHeight-1;
                int counter=0;int black=0;
                for(int i=iterate.getX();i<=imax && i<=upRight.getX();i++)
                    for(int j=iterate.getY();j<=jmax && j<=upRight.getY();j++) {
                        if(boolImg[i][j]==true) black++;
                        counter++;
                    }
                //Count percentage
                if(black/counter>percentage){
                    newBoolImg[row][col]=true;
                }else newBoolImg[row][col]=false;
            }
        }
        return newBoolImg;
    }

    private PairPoint findRectangle(ChainCodeGenerator.BorderInfo border,boolean[][] boolImg){
        String chaincodes=border.chainCodes;
        Point p=border.startPoint;
        int up=p.getY(),right=p.getX(),bottom=p.getY(),left=p.getX();
        for(int i=0;i<chaincodes.length();i++){
            p=chcodeToPoint(p,chaincodes.charAt(i));
            if(p.getX()>right)right=p.getX();
            if(p.getX()<left)left=p.getX();
            if(p.getY()>up)up=p.getY();
            if(p.getY()<bottom) bottom=p.getY();
        }

        return  new PairPoint(new Point(left,bottom),new Point(right,up));

    }

    public List<ChainCodeGenerator.BorderInfo> doNxN(NativeBitmap src){
        int width=src.width;
        int height=src.height;
        List<ChainCodeGenerator.BorderInfo> bordersTrans=new ArrayList<>();
        List<ChainCodeGenerator.BorderInfo> temp;
        boolean[][] boolImg=src.convertToBoolmage();
        ChainCodeGenerator ccg = new ChainCodeGenerator();
        List<ChainCodeGenerator.BorderInfo> bordersInfo=ccg.getBorderInfos(boolImg, width, height);

        for(ChainCodeGenerator.BorderInfo border: bordersInfo){
            boolean[][] array=singleToNxN(border,boolImg);
            temp = ccg.getBorderInfos(array, N + 2, N + 2);
            if(temp.size()>0) //chaincode exist
                bordersTrans.add(temp.get(0));
        }
        return bordersTrans;
    }
}
