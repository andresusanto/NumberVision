package com.ganesus.numbervision;

import android.graphics.Point;

import java.util.List;
import java.util.Queue;
import java.util.concurrent.SynchronousQueue;

/**
 * Created by Andre on 10/13/2015.
 */
public class ChainGenerator {
    public static class BorderInfo{
        Point start_point;
        List<Integer> chain_codes;
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



}
