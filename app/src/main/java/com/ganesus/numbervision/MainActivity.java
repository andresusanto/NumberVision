package com.ganesus.numbervision;

import android.content.Intent;
import android.database.Cursor;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.net.Uri;
import android.provider.MediaStore;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.view.View;
import android.widget.ImageView;
import android.widget.TextView;

import org.w3c.dom.Text;

public class MainActivity extends AppCompatActivity {

    private static final int RESULT_LOAD_IMAGE = 635;

    public void klikGallery(View v){
        Intent i = new Intent(Intent.ACTION_PICK, android.provider.MediaStore.Images.Media.EXTERNAL_CONTENT_URI);
        startActivityForResult(i, RESULT_LOAD_IMAGE);
    }

    private void prosesBitmap(String path){
        //Bitmap bmp = Bitmap.createBitmap(BitmapFactory.decodeFile(path), 18, 37, 918, 128);
        Bitmap bmp = BitmapFactory.decodeFile(path);

        if (bmp.getWidth() == 1632){
            bmp = Bitmap.createBitmap(bmp, 546, 491, 549, 115);
        }else if(bmp.getWidth() == 1024){
            bmp = Bitmap.createBitmap(bmp, 345, 323, 390, 107);
        }

        String hasil = detectAll(bmp)[0];

        Bitmap.Config conf = Bitmap.Config.ARGB_8888;
        Bitmap canvas = Bitmap.createBitmap(bmp.getWidth(), bmp.getHeight(), conf);

        Bitmap hh = preProses(bmp, canvas);

        TextView tv = (TextView) findViewById(R.id.txtInterpretasi);
        tv.setText(hasil);

        ImageView iv = (ImageView) findViewById(R.id.imageView);
        iv.setImageBitmap(hh);
        //iv.setImageBitmap(BitmapFactory.decodeFile(path));
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);



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


            prosesBitmap(picturePath);

        }

    }

    public native String[] detectAll(Bitmap bitmap);
    public native Bitmap preProses(Bitmap bitmap, Bitmap canvas);

    static {
        System.loadLibrary("numbervision");
    }
}
