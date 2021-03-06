package com.ganesus.numbervision;

import android.content.Intent;
import android.database.Cursor;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.net.Uri;
import android.os.Bundle;
import android.provider.MediaStore;
import android.support.v7.app.AppCompatActivity;
import android.util.Log;
import android.view.View;

import com.ganesus.numbervision.engine.ChainCodeGenerator;
import com.ganesus.numbervision.engine.Interpretator;
import com.ganesus.numbervision.engine.NativeBitmap;
import com.ganesus.numbervision.engine.ToNxN;

import java.util.List;

public class VisionActivity extends AppCompatActivity {
    private static final int RESULT_LOAD_IMAGE = 1212;
    private Interpretator interpretator;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_vision);

        interpretator = new Interpretator(getResources(), R.raw.knowledge);
    }

    public void onClickTest(View v){
        Intent i = new Intent(Intent.ACTION_PICK, android.provider.MediaStore.Images.Media.EXTERNAL_CONTENT_URI);
        startActivityForResult(i, RESULT_LOAD_IMAGE);
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        super.onActivityResult(requestCode, resultCode, data);

        if (requestCode == RESULT_LOAD_IMAGE && resultCode == RESULT_OK && null != data) {
            Uri selectedImage = data.getData();
            String[] filePathColumn = {MediaStore.Images.Media.DATA};

            Cursor cursor = getContentResolver().query(selectedImage, filePathColumn, null, null, null);
            cursor.moveToFirst();

            int columnIndex = cursor.getColumnIndex(filePathColumn[0]);
            String picturePath = cursor.getString(columnIndex);
            cursor.close();

            Bitmap bmp = BitmapFactory.decodeFile(picturePath);
            NativeBitmap nativeBitmap = new NativeBitmap(bmp);
            nativeBitmap.grayscaleBitmap();

            int w = bmp.getWidth(); int h = bmp.getHeight();
            bmp.recycle();

            boolean[][] boolImage = nativeBitmap.convertToBoolmage();
            boolean[][] boolImage2 = nativeBitmap.convertToBoolmage();


            /*//print image before thinning
            Log.d("DEBUG","Before Thinning");
            for (int i=0;i<h;i++) {
                StringBuilder line = new StringBuilder("");
                for (int j=0;j<w;j++) {
                    if (boolImage[i][j]) line.append("1");
                    else line.append("0");
                }
                Log.d("DEBUG",line.toString());
            }

            ZhangSuenGenerator zhangSuenGenerator = new ZhangSuenGenerator();
            zhangSuenGenerator.doZhangSuenThinning(boolImage,true);

            //print image after thinning
            Log.d("DEBUG","ZhangSuenThinning");
            for (int i=0;i<h;i++) {
                StringBuilder line = new StringBuilder("");
                for (int j=0;j<w;j++) {
                    if (boolImage[i][j]) line.append("1");
                    else line.append("0");
                }
                Log.d("DEBUG",line.toString());
            }*/

            ChainCodeGenerator ccg = new ChainCodeGenerator();
            List<ChainCodeGenerator.BorderInfo> borderInfos = ccg.getBorderInfos(boolImage,w,h);

            ToNxN compressor = new ToNxN(5);

            for (int i = 0 ; i < borderInfos.size(); i++){
                Log.i("NUMVISION", borderInfos.get(i).chainCodes);
                if (borderInfos.get(i).chainCodes.length() > 10) {
                    boolean[][] compressImage = compressor.singleToNxN(borderInfos.get(i),boolImage2);

                    //Ingat ukuran akhir adalah (N + 2) * (N + 2)
                    for (int j=0;j<7;j++) {
                        StringBuilder line = new StringBuilder("");
                        for (int k=0;k<7;k++) {
                            if (compressImage[j][k]) line.append("1");
                            else line.append("0");
                        }
                        Log.d("DEBUG",line.toString());
                    }

                }


            }
            Log.d("DEBUG","Sudah selesai");
        }
    }
}
