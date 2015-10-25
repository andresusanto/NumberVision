package com.ganesus.numbervision.engine;

import java.util.LinkedList;
import java.util.List;

public class ZhangSuenGenerator {

    public boolean[][] doZhangSuenThinning(final boolean[][] givenImage, boolean changeGivenImage) {
        boolean[][] binaryImage;
        if (changeGivenImage) {
            binaryImage = givenImage;
        } else {
            binaryImage = givenImage.clone();
        }
        int a, b;
        List<Point> pointsToChange = new LinkedList();
        boolean hasChange;
        do {
            hasChange = false;
            for (int y = 1; y + 1 < binaryImage.length; y++) {
                for (int x = 1; x + 1 < binaryImage[y].length; x++) {
                    a = getA(binaryImage, y, x);
                    b = getB(binaryImage, y, x);
                    if (binaryImage[y][x] == true && 2 <= b && b <= 6 && a == 1
                            && (binaryImage[y - 1][x] == false || binaryImage[y][x + 1] == false || binaryImage[y + 1][x] == false)
                            && (binaryImage[y][x + 1] == false || binaryImage[y + 1][x] == false || binaryImage[y][x - 1] == false)) {
                        pointsToChange.add(new Point(x, y));
                        hasChange = true;
                    }
                }
            }
            for (Point point : pointsToChange) {
                binaryImage[point.getY()][point.getX()] = false;
            }
            pointsToChange.clear();
            for (int y = 1; y + 1 < binaryImage.length; y++) {
                for (int x = 1; x + 1 < binaryImage[y].length; x++) {
                    a = getA(binaryImage, y, x);
                    b = getB(binaryImage, y, x);
                    if (binaryImage[y][x] == true && 2 <= b && b <= 6 && a == 1
                            && (binaryImage[y - 1][x] == false || binaryImage[y][x + 1] == false || binaryImage[y][x - 1] == false)
                            && (binaryImage[y - 1][x] == false || binaryImage[y + 1][x] == false || binaryImage[y][x - 1] == false)) {
                        pointsToChange.add(new Point(x, y));
                        hasChange = true;
                    }
                }
            }
            for (Point point : pointsToChange) {
                binaryImage[point.getY()][point.getX()] = false;
            }
            pointsToChange.clear();
        } while (hasChange);
        return binaryImage;
    }

    private int getA(boolean[][] binaryImage, int y, int x) {
        int count = 0;
        if (y - 1 >= 0 && x + 1 < binaryImage[y].length && binaryImage[y - 1][x] == false && binaryImage[y - 1][x + 1] == true) {
            count++;
        }
        if (y - 1 >= 0 && x + 1 < binaryImage[y].length && binaryImage[y - 1][x + 1] == false && binaryImage[y][x + 1] == true) {
            count++;
        }
        if (y + 1 < binaryImage.length && x + 1 < binaryImage[y].length && binaryImage[y][x + 1] == false && binaryImage[y + 1][x + 1] == true) {
            count++;
        }
        if (y + 1 < binaryImage.length && x + 1 < binaryImage[y].length && binaryImage[y + 1][x + 1] == false && binaryImage[y + 1][x] == true) {
            count++;
        }
        if (y + 1 < binaryImage.length && x - 1 >= 0 && binaryImage[y + 1][x] == false && binaryImage[y + 1][x - 1] == true) {
            count++;
        }
        if (y + 1 < binaryImage.length && x - 1 >= 0 && binaryImage[y + 1][x - 1] == false && binaryImage[y][x - 1] == true) {
            count++;
        }
        if (y - 1 >= 0 && x - 1 >= 0 && binaryImage[y][x - 1] == false && binaryImage[y - 1][x - 1] == true) {
            count++;
        }
        if (y - 1 >= 0 && x - 1 >= 0 && binaryImage[y - 1][x - 1] == false && binaryImage[y - 1][x] == true) {
            count++;
        }
        return count;
    }

    private int getB(boolean[][] binaryImage, int y, int x) {
        int accumulator = 0;
        Point point = new Point(x,y);
        for (int i=0;i<8;i++) {
            Point currentPoint = point.add(Point.direction[i]);
            if (binaryImage[currentPoint.y][currentPoint.x]) accumulator+=1;
        }
        return accumulator;
    }
}

