package com.ganesus.numbervision;

import android.graphics.Bitmap;

/**
 * Created by Andre on 10/13/2015.
 */
public class NativeBitmap {
    public int pixels[];
    public int width;
    public int height;


    public static class RGB{
        int alpha, red, green, blue;
    };

    public NativeBitmap(int width, int height) {
        pixels = new int[width * height];
        this.width = width;
        this.height = height;
    }

    private int convertArgbToInt(RGB argb) {
        return 0xFF000000 | (argb.red << 16) | (argb.green << 8) | (argb.blue);
    }

    private RGB convertIntToArgb(int pixel){
        RGB ret = new RGB();

        ret.red = ((pixel >> 16) & 0xff);
        ret.green = ((pixel >> 8) & 0xff);
        ret.blue = ((pixel) & 0xff);

        return ret;
    }

    public void grayscaleBitmap(){
        int nBitmapSize = width * height;

        for (int i = 0; i < nBitmapSize; i++){
            RGB bitmapColor = convertIntToArgb(pixels[i]);

            int grayscaleColor = (int)(0.2989f * bitmapColor.red + 0.5870f * bitmapColor.green + 0.1141 * bitmapColor.blue);

            bitmapColor.red = grayscaleColor;
            bitmapColor.green = grayscaleColor;
            bitmapColor.blue = grayscaleColor;

            pixels[i] = convertArgbToInt(bitmapColor);
        }

    }

    public int[] createHistogram(){
        int result[] = new int[256];
        int nBitmapSize = width * height;

        for (int i = 0; i < nBitmapSize; i++){
            RGB bitmapColor = convertIntToArgb(pixels[i]);
            result[bitmapColor.red]++;
        }

        return result;
    }

    public float generateOtsu(int histogram[], int total) {
        int sum = 0;
        for (int i=1;i<256; ++i) sum+= i * histogram[i];

        int sumB = 0;
        int wB = 0;
        int wF = 0;
        int mB = 0;
        int mF = 0;
        float max = 0.0f;
        float between = 0.0f;
        float threshold1 = 0.0f;
        float threshold2 = 0.0f;

        for (int i=0;i<256;++i) {
            wB += histogram[i];
            if (wB == 0) continue;
            wF = total - wB;
            if (wF == 0) break;

            sumB += i * histogram[i];

            mB = sumB / wB;
            mF = (sum - sumB) /wF;

            between = wB * wF * (mB - mF) * (mB - mF);
            if (between >= max) {
                threshold1 = i;
                if ( between > max ) {
                    threshold2 = i;
                }
                max = between;
            }
        }

        return (threshold1 + threshold2) / 2.0f;
    }

    public int[][] convertToBinary(){
        int image[][] = new int[height][];
        int nBitmapSize = width * height;

        int histogram[] = createHistogram();

        float otsu = generateOtsu(histogram, nBitmapSize);


        for (int i=0;i< height;i++) {
            image[i] = new int[width];

            for (int j=0;j< width;j++) {
                RGB warna = convertIntToArgb(pixels[i * width + j]);
                if (warna.red > otsu){
                    image[i][j] = 1;
                }
            }
        }

        // create border
        for (int i=0;i<width;i++) {
            image[0][i] = 0;
            image[height-1][i] = 0;
        }

        for (int i=0;i<height;i++) {
            image[i][0] = 0;
            image[i][width-1] = 0;
        }

        return image;
    }

    public boolean[][] convertToBoolmage(){ // syarat harus grayscale
        boolean image[][] = new boolean[height][];
        int nBitmapSize = width * height;

        int histogram[] = createHistogram();

        float otsu = generateOtsu(histogram, nBitmapSize);


        for (int i=0;i< height;i++) {
            image[i] = new boolean[width];

            for (int j=0;j< width;j++) {
                RGB warna = convertIntToArgb(pixels[i * width + j]);
                image[i][j] = (warna.red < otsu);
            }
        }

        // create border
        for (int i=0;i<width;i++) {
            image[0][i] = false;
            image[height-1][i] = false;
        }

        for (int i=0;i<height;i++) {
            image[i][0] = false;
            image[i][width-1] = false;
        }

        return image;
    }

    NativeBitmap(Bitmap bitmap){
        width = bitmap.getWidth();
        height = bitmap.getHeight();
        pixels = new int[bitmap.getWidth() * bitmap.getHeight()];

        bitmap.getPixels(pixels, 0, bitmap.getWidth(), 0, 0, bitmap.getWidth(), bitmap.getHeight());
    }

    public void smooth() {
        int _pixels[] = new int[width * height];

        for (int i=0;i<height;i++) {
            for (int j=0;j<width;j++) {
                Point currentPoint = new Point(j,i);
                int nNeighbor = 0;
                RGB rgbAccum = new RGB();
                for (int k=0;k<8;k++) {
                    Point neighbor = currentPoint.add(Point.direction[k]);
                    if ((neighbor.x > 0) && (neighbor.y > 0) && (neighbor.x < width) && (neighbor.y < height)) {
                        nNeighbor++;
                        RGB currentRGB = convertIntToArgb(pixels[i * width + j]);
                        rgbAccum.red += currentRGB.red;
                        rgbAccum.green += currentRGB.green;
                        rgbAccum.blue += currentRGB.blue;
                    }
                }
                rgbAccum.red /= nNeighbor;
                rgbAccum.green /= nNeighbor;
                rgbAccum.blue /= nNeighbor;
                _pixels[i * width + j] = convertArgbToInt(rgbAccum);
            }
        }
        this.pixels = _pixels;
    }



}
