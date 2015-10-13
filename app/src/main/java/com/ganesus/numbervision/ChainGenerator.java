package com.ganesus.numbervision;

import android.graphics.Point;

import java.util.ArrayList;
import java.util.LinkedList;
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

        BorderInfo(){
            start_point = new Point();
            chain_codes = new ArrayList<>();
        }
    }

    private Point sumPoint(Point a, Point b){
        Point ret = new Point(a);
        a.set(a.x + b.x, a.y + b.y);

        return ret;
    }

    private Point get_next_traverse_point(Point current_black,Point current_traverse_point) {
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

    private int get_chain_code(Point current_black,Point prev_black) {
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


    private void erase_image(Point start_point, boolean image[][],int length,int height) {

        Point direction[] = {new Point(1,0), new Point(1,1), new Point(0,1), new Point(-1,1), new Point(-1,0), new Point(-1,-1), new Point(0,-1), new Point(1,-1)};

        Queue<Point> bfs_queue = new LinkedList<>();

        bfs_queue.add(start_point);
        image[start_point.y][start_point.x] = false;


        while(!bfs_queue.isEmpty()) {
            Point front = bfs_queue.poll();

            for (int i=0;i<8;i++) {
                Point current_point = sumPoint(front , direction[i]);
                if (current_point.x >=0 && current_point.x <= length && current_point.y >= 0 && current_point.y <= height && image[current_point.y][current_point.x]) {
                    image[current_point.y][current_point.x] = false;
                    bfs_queue.add(current_point);
                }
            }
        }
    }

    private boolean is_point(Point point, boolean image[][]) {

        int x = point.x;
        int y = point.y;

        return image[y][x] && !image[y-1][x] && !image[y][x-1] && !image[y+1][x] && !image[y][x+1];
    }

    private Point get_start_point(boolean image[][], int length,int height) {
        for (int i=0;i<height;i++) {
            for (int j=0;j<length;j++) {
                if (image[i][j]) return new Point(j,i);
            }
        }
        return new Point(-1,-1);
    }

    private List<Integer> get_chain_codes(Point start_point, boolean image[][],int length,int height) {

        List<Integer> chain_codes = new ArrayList<>();

        if (is_point(start_point,image)) return chain_codes;

        Point current_black = start_point;
        Point current_white = current_black;
        current_white.x -= 1;

        Point black0 = current_black;
        Point black1 = new Point();

        boolean has_first_found = false;

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

                chain_codes.add(chain_code);
                current_black = traverse_point;
                current_white = traverse_point_prev;
                traverse_point = current_white;

            }
        }
        return chain_codes;
    }

    List<BorderInfo> get_border_infos(boolean image[][],int length,int height) {
        List<BorderInfo> border_infos = new ArrayList<>();

        while(true) {
            Point start_point = get_start_point(image, length, height);
            if (start_point.x == -1 && start_point.y == -1) {
                break;
            }
            BorderInfo border_info = new BorderInfo();
            border_info.start_point = start_point;
            border_info.chain_codes = get_chain_codes(start_point, image, length, height);

            border_infos.add(border_info);
            erase_image(start_point,image,length,height);
        }
        return border_infos;
    }
    

}
