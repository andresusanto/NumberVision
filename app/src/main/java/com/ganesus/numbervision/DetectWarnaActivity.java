package com.ganesus.numbervision;

import android.content.Intent;
import android.database.Cursor;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.net.Uri;
import android.os.Environment;
import android.provider.MediaStore;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.Log;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.webkit.WebView;

import java.io.File;
import java.io.IOException;
import java.text.SimpleDateFormat;
import java.util.Collections;
import java.util.Comparator;
import java.util.Date;
import java.util.HashMap;
import java.util.Iterator;
import java.util.LinkedHashMap;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;

public class DetectWarnaActivity extends AppCompatActivity {
    private static final int RESULT_LOAD_IMAGE = 635;
    private static final int REQUEST_IMAGE_CAPTURE = 695;

    private String mCurrentPhotoPath;

    private File createImageFile() throws IOException {
        String timeStamp = new SimpleDateFormat("yyyyMMdd_HHmmss").format(new Date());
        String imageFileName = "JPEG_" + timeStamp + "_";
        File storageDir = Environment.getExternalStoragePublicDirectory(Environment.DIRECTORY_PICTURES);
        storageDir.mkdirs();
        File image = File.createTempFile(
                imageFileName,
                ".jpg",
                storageDir
        );

        mCurrentPhotoPath = image.getAbsolutePath();
        return image;
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_detect_warna);

        WebView wv = (WebView) findViewById(R.id.webView);
        wv.loadData("", "text/html", "UTF-8");
    }

    public void onClickGallery(View v) {
        Intent i = new Intent(Intent.ACTION_PICK, android.provider.MediaStore.Images.Media.EXTERNAL_CONTENT_URI);
        startActivityForResult(i, RESULT_LOAD_IMAGE);
    }

    public void onClickCamera(View v){
        Intent takePictureIntent = new Intent(MediaStore.ACTION_IMAGE_CAPTURE);
        if (takePictureIntent.resolveActivity(getPackageManager()) != null) {
            File photoFile = null;
            try {
                photoFile = createImageFile();
            } catch (IOException ex) {

            }

            if (photoFile != null) {
                takePictureIntent.putExtra(MediaStore.EXTRA_OUTPUT, Uri.fromFile(photoFile));
                startActivityForResult(takePictureIntent, REQUEST_IMAGE_CAPTURE);
            }

        }
    }

    private void prosesBitmap(String picturePath){
        Bitmap bmp = Bitmap.createScaledBitmap(BitmapFactory.decodeFile(picturePath), 500, 500, false);

        int ukuran = bmp.getWidth() * bmp.getHeight();
        int[] pixels = new int[ukuran];
        bmp.getPixels(pixels, 0, bmp.getWidth(), 0, 0, bmp.getWidth(), bmp.getHeight());
        bmp.recycle();

        String color = "";
        Map<String, Long> map = new HashMap<String, Long>();

        for (int i = 0 ; i < ukuran; i++){
            int pixel = pixels[i];
            int R1 = (pixel >> 16) & 0xff;
            int G1 = (pixel >> 8) & 0xff;
            int B1 = (pixel & 0xff);

            String pad1 = "", pad2 = "", pad3 = "";
            if (R1 < 16) pad1 = "0";
            if (G1 < 16) pad2 = "0";
            if (B1 < 16) pad3 = "0";


            String warna = pad1 + Integer.toHexString(R1) + pad2 + Integer.toHexString(G1) + pad3 + Integer.toHexString(B1);
            if (!map.containsKey(warna)){
                map.put(warna, 1l);
                //color = color + "<font color='#" + warna + ">#" + warna + "</font><br/>\n";
            }else{
                map.put(warna, ((long)map.get(warna) + 1l));
            }
        }

        pixels = null;

        Map<String, Long> sortedMap = sortByComparator(map);

        int i = 0;
        for (Map.Entry<String, Long> entry : sortedMap.entrySet()) {
            String key = entry.getKey();
            color = color + "<span style='background-color: #"+key+";'>&nbsp;&nbsp;&nbsp;&nbsp;</span> &nbsp; #" + key + " = "+entry.getValue()+"<br/>\n";
            i++;
            if (i == 100) break;
        }

        WebView wv = (WebView) findViewById(R.id.webView);
        wv.getSettings().setDomStorageEnabled(true);
        wv.loadDataWithBaseURL("", "<img src='file://" + picturePath+"' style='max-width: 100%;'/><br/>Warna yang ditemukan: " + map.size() + "<br/><br/>TOP 100 Warna:<br/>" + color, "text/html", "utf-8", "");

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

        }else if (requestCode == REQUEST_IMAGE_CAPTURE && resultCode == RESULT_OK) {
            Log.d("ANJING", mCurrentPhotoPath);
            prosesBitmap(mCurrentPhotoPath);
        }

    }

    private static Map<String, Long> sortByComparator(Map<String, Long> unsortMap) {
        List<Map.Entry<String, Long>> list = new LinkedList<Map.Entry<String, Long>>(unsortMap.entrySet());
        Collections.sort(list, new Comparator<Map.Entry<String, Long>>() {
            public int compare(Map.Entry<String, Long> o1,
                               Map.Entry<String, Long> o2) {
                return -1 * (o1.getValue()).compareTo(o2.getValue());
            }
        });

        Map<String, Long> sortedMap = new LinkedHashMap<String, Long>();
        for (Iterator<Map.Entry<String, Long>> it = list.iterator(); it.hasNext();) {
            Map.Entry<String, Long> entry = it.next();
            sortedMap.put(entry.getKey(), entry.getValue());
        }
        return sortedMap;
    }


}
