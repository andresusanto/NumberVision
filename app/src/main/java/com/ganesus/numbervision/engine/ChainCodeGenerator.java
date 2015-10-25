package com.ganesus.numbervision.engine;

import java.util.ArrayList;
import java.util.LinkedList;
import java.util.List;
import java.util.Queue;

/**
 * Created by kevinyu on 10/12/15.
 */
public class ChainCodeGenerator {

    public static class BorderInfo {
        public Point startPoint;
        public String chainCodes;
    }

    private Point getNextTraversePoint(Point currentBlack,Point currentTraversePoint) {
        Point next = new Point (currentTraversePoint.x,currentTraversePoint.y);
        if (currentTraversePoint.x == currentBlack.x-1 &&
                currentTraversePoint.y == currentBlack.y+1) {
            next.y -= 1;
        } else if (currentTraversePoint.x == currentBlack.x-1 &&
                currentTraversePoint.y == currentBlack.y) {
            next.y -= 1;
        } else if (currentTraversePoint.x == currentBlack.x-1 &&
                currentTraversePoint.y == currentBlack.y-1) {
            next.x += 1;
        } else if (currentTraversePoint.x == currentBlack.x &&
                currentTraversePoint.y == currentBlack.y-1) {
            next.x += 1;
        } else if (currentTraversePoint.x == currentBlack.x + 1 &&
                currentTraversePoint.y == currentBlack.y-1) {
            next.y += 1;
        } else if (currentTraversePoint.x == currentBlack.x + 1 &&
                currentTraversePoint.y == currentBlack.y) {
            next.y += 1;
        } else if (currentTraversePoint.x == currentBlack.x + 1 &&
                currentTraversePoint.y == currentBlack.y + 1) {
            next.x -= 1;
        } else if (currentTraversePoint.x == currentBlack.x &&
                currentTraversePoint.y == currentBlack.y + 1) {
            next.x -= 1;
        }
        return next;
    }

    private String getChainCode(Point currentBlack,Point prevBlack) {
        String chain_code = "0";
        if (prevBlack.x == currentBlack.x-1 &&
                prevBlack.y == currentBlack.y+1) {
            chain_code = "7";
        } else if (prevBlack.x == currentBlack.x-1 &&
                prevBlack.y == currentBlack.y) {
            chain_code = "0";
        } else if (prevBlack.x == currentBlack.x-1 &&
                prevBlack.y == currentBlack.y-1) {
            chain_code = "1";
        } else if (prevBlack.x == currentBlack.x &&
                prevBlack.y == currentBlack.y-1) {
            chain_code = "2";
        } else if (prevBlack.x == currentBlack.x + 1 &&
                prevBlack.y == currentBlack.y-1) {
            chain_code = "3";
        } else if (prevBlack.x == currentBlack.x + 1 &&
                prevBlack.y == currentBlack.y) {
            chain_code = "4";
        } else if (prevBlack.x == currentBlack.x + 1 &&
                prevBlack.y == currentBlack.y + 1) {
            chain_code = "5";
        } else if (prevBlack.x == currentBlack.x &&
                prevBlack.y == currentBlack.y + 1) {
            chain_code = "6";
        }
        return chain_code;
    }

    void eraseImage(Point startPoint,
                     boolean [][]image,int length,int height) {

        Queue<Point> bfs_queue = new LinkedList<>();
        bfs_queue.add(startPoint);
        image[startPoint.y][startPoint.x] = false;

        while(!bfs_queue.isEmpty()) {
            Point front = bfs_queue.poll();

            for (int i=0;i<8;i++) {
                Point tmp = front.add(Point.direction[i]);
                Point current_point = new Point(tmp.x,tmp.y);
                if (current_point.x >=0 && current_point.x <= length && current_point.y >= 0 && current_point.y <= height &&
                        image[current_point.y][current_point.x]) {
                    image[current_point.y][current_point.x] = false;
                    bfs_queue.add(current_point);
                }
            }
        }
    }

    private boolean isPoint (Point point, boolean[][] image) {
        int x = point.x;
        int y = point.y;

        return image[y][x] && !image[y-1][x] && !image[y][x-1] && !image[y+1][x] && !image[y][x+1];
    }

    private Point getStartPoint(boolean[][] image, int length,int height) {
        for (int i=0;i<height;i++) {
            for (int j=0;j<length;j++) {
                if (image[i][j]) return new Point(j,i);
            }
        }
        return new Point(-1,-1);
    }

    public String get_chain_codes(Point startPoint,
                                boolean [][]image,int length,int height) {

        StringBuilder chain_codes = new StringBuilder("");

        if (isPoint(startPoint, image)) return chain_codes.toString();

        Point currentBlack = new Point(startPoint.x,startPoint.y);
        Point current_white = new Point(currentBlack.x,currentBlack.y);
        current_white.x -= 1;

        Point black0 = new Point(currentBlack.x, currentBlack.y);
        Point black1 = null;

        boolean has_first_found = false;

        Point traverse_point = new Point(current_white.x,current_white.y);
        Point traverse_point_prev = new Point(current_white.x,current_white.y);

        while (true) {
            traverse_point_prev.setPoint(traverse_point);

            traverse_point.setPoint(getNextTraversePoint(currentBlack, traverse_point));
            if (has_first_found && currentBlack.equals(black0) && black1.equals(traverse_point)) {
                break;
            }

            if (image[traverse_point.y][traverse_point.x]) {
                String chain_code = getChainCode(traverse_point,currentBlack);

                if (!has_first_found) {
                    black1 = new Point(traverse_point.x,traverse_point.y);
                    has_first_found = true;
                }

                chain_codes.append(chain_code);
                currentBlack.setPoint(traverse_point);
                current_white.setPoint(traverse_point_prev);
                traverse_point.setPoint(current_white);

            }
        }
        return chain_codes.toString();
    }

    public ArrayList<BorderInfo> getBorderInfos(boolean[][] image, int length, int height) {
        ArrayList<BorderInfo> borderInfos = new ArrayList<>();
        while(true) {
            Point startPoint = getStartPoint(image, length, height);
            if (startPoint.x == -1 && startPoint.y == -1) {
                break;
            }
            BorderInfo border_info = new BorderInfo();
            border_info.startPoint = startPoint;
            border_info.chainCodes = get_chain_codes(startPoint, image, length, height);

            borderInfos.add(border_info);
            eraseImage(startPoint, image, length, height);
        }
        return borderInfos;
    }

    public String generateSingle(boolean[][] image, int length, int height){
        List<BorderInfo> borderInfos = getBorderInfos(image, length, height);
        if (borderInfos.size() > 0){
            return borderInfos.get(0).chainCodes;
        }else{
            return "";
        }
    }
}