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

import java.util.List;

public class VisionActivity extends AppCompatActivity {

    private static int RESULT_LOAD_IMAGE = 1212;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_vision);
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


            ChainCodeGenerator ccg = new ChainCodeGenerator();

            Log.d("DEBUG","d1");
            List<ChainCodeGenerator.BorderInfo> borderInfos = ccg.getBorderInfos(boolImage,w,h);

            Log.d("DEBUG","d2");
            for (int i = 0 ; i < borderInfos.size(); i++){
                StringBuffer sb = new StringBuffer();
                for (int j = 0 ; j < borderInfos.get(i).chainCodes.length(); j++){
                    sb.append(borderInfos.get(i).chainCodes.charAt(j));
                }

                Log.i("NUMVISION", sb.toString());
            }

            Log.d("DEBUG","Sudah selesai");
        }

    }
}
