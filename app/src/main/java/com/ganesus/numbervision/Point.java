package com.ganesus.numbervision;

/**
 * Created by kevinyu on 10/14/15.
 */
public class Point {
    public int x;
    public int y;

    public Point(int x, int y) {
        this.x = x;
        this.y = y;
    }

    public int getX() {
        return x;
    }

    public void setX(int x) {
        this.x = x;
    }

    public int getY() {
        return y;
    }

    public void setY(int y) {
        this.y = y;
    }

    public void setPoint(Point point) {
        this.x = point.x;
        this.y = point.y;
    }

    Point add(Point other) {
        return new Point(x + other.x, y + other.y);
    }

    public boolean equals(Point point) {
        return x == point.x && y == point.y;
    }
}