package com.ganesus.numbervision;

import android.content.Context;
import android.content.Intent;
import android.content.res.Resources;
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

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;

public class MainActivity extends AppCompatActivity {

    private static final int RESULT_LOAD_IMAGE = 635;
    private static String KNOWLEDGE_PATH = "";

    public static File copyResource (Resources r, int rsrcId, File dstFile) throws IOException
    {
        InputStream is =  r.openRawResource(rsrcId);
        FileOutputStream os = new FileOutputStream(dstFile);

        byte[] buffer = new byte[4096];
        int bytesRead;
        while ((bytesRead = is.read(buffer)) != -1)
            os.write(buffer, 0, bytesRead);
        is.close();
        os.close();

        return dstFile;
    }

    public void klikGallery(View v){
        Intent i = new Intent(Intent.ACTION_PICK, android.provider.MediaStore.Images.Media.EXTERNAL_CONTENT_URI);
        startActivityForResult(i, RESULT_LOAD_IMAGE);
    }

    private void prosesBitmap(String path){
        Bitmap merek = null;
        Bitmap bmp = BitmapFactory.decodeFile(path);

        if (bmp.getWidth() == 1632){
            merek = Bitmap.createBitmap(bmp, 644, 303, 346, 112);
            bmp = Bitmap.createBitmap(bmp, 546, 491, 549, 115);
        }else if(bmp.getWidth() == 1024){
            bmp = Bitmap.createBitmap(bmp, 345, 323, 390, 107);
        }else if(bmp.getWidth() == 1019){
            merek = Bitmap.createBitmap(bmp, 382, 302, 247, 93);
            bmp = Bitmap.createBitmap(bmp, 341, 438, 332, 55);
        }



        String hasil = detectAll(bmp, KNOWLEDGE_PATH)[0];

        Bitmap.Config conf = Bitmap.Config.ARGB_8888;
        Bitmap canvas = Bitmap.createBitmap(bmp.getWidth(), bmp.getHeight(), conf);

        Bitmap hh = preProses(bmp, canvas);

        if (merek != null){
            Bitmap p_merek = preProses(bmp, canvas);

            Bitmap.Config conf_merek = Bitmap.Config.ARGB_8888;
            Bitmap canvas_merek = Bitmap.createBitmap(merek.getWidth(), merek.getHeight(), conf_merek);
            Bitmap hmerek = preProses(merek, canvas_merek);

            ImageView iv_merek = (ImageView) findViewById(R.id.imageView2);
            iv_merek.setImageBitmap(hmerek);

            String merekmobil = detectMerek(merek)[0];
            TextView tv = (TextView) findViewById(R.id.txtPerhitungan);
            tv.setText(merekmobil);

        }else{
            ImageView iv_merek = (ImageView) findViewById(R.id.imageView2);
            iv_merek.setImageBitmap(null);

            TextView tv = (TextView) findViewById(R.id.txtPerhitungan);
            tv.setText("Tidak diketahui");
        }

        TextView tv = (TextView) findViewById(R.id.txtInterpretasi);
        tv.setText(hasil);

        ImageView iv = (ImageView) findViewById(R.id.imageView);
        iv.setImageBitmap(hh);
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        File dstDir = getDir("data", Context.MODE_PRIVATE);
        File knowledgeFile = new File(dstDir, "knowledge.txt");

        try {
            copyResource(getResources(), R.raw.knowledge, knowledgeFile);
        } catch (IOException e) {
            e.printStackTrace();
        }
        KNOWLEDGE_PATH = knowledgeFile.getAbsolutePath();
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

    public native String[] detectAll(Bitmap bitmap, String knowledge);
    public native String[] detectMerek(Bitmap bitmap);
    public native Bitmap preProses(Bitmap bitmap, Bitmap canvas);

    static {
        System.loadLibrary("numbervision");
    }
}
