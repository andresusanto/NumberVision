package com.ganesus.numbervision;

import java.util.ArrayList;
import java.util.LinkedList;
import java.util.Queue;

/**
 * Created by kevinyu on 10/12/15.
 */
public class ChainCodeGenerator {

    public static class BorderInfo {
        Point startPoint;
        String chainCodes;
    }

    private Point getNextTraversePoint(Point currentBlack,Point currentTraversePoint) {
        Point next = currentTraversePoint;
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

    private static Point[] direction = {new Point(1,0), new Point(1,1), new Point(0,1), new Point(-1,1),
            new Point(-1,0), new Point(-1,-1), new Point(0,-1), new Point(1,-1)};

    void eraseImage(Point startPoint,
                     boolean [][]image,int length,int height) {

        Queue<Point> bfs_queue = new LinkedList<>();
        bfs_queue.add(startPoint);
        image[startPoint.y][startPoint.x] = false;

        while(!bfs_queue.isEmpty()) {
            Point front = bfs_queue.poll();

            for (int i=0;i<8;i++) {
                Point current_point = front.add(direction[i]);
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

        Point currentBlack = startPoint;
        Point current_white = currentBlack;
        current_white.x -= 1;

        Point black0 = currentBlack;
        Point black1 = null;

        boolean has_first_found = false;

        Point traverse_point = current_white;
        Point traverse_point_prev = current_white;
        while (true) {

            traverse_point_prev = traverse_point;
            traverse_point = getNextTraversePoint(currentBlack, traverse_point);
            if (has_first_found && currentBlack == black0 && black1 == traverse_point) {
                break;
            }
            if (image[traverse_point.y][traverse_point.x]) {
                String chain_code = getChainCode(traverse_point,currentBlack);

                if (!has_first_found) {
                    black1 = traverse_point;
                    has_first_found = true;
                }

                chain_codes.append(chain_code);
                currentBlack = traverse_point;
                current_white = traverse_point_prev;
                traverse_point = current_white;

            }
        }
        return chain_codes.toString();
    }

    ArrayList<BorderInfo> getBorderInfos(boolean[][] image, int length, int height) {
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


}