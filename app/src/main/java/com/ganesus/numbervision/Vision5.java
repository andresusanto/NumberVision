package com.ganesus.numbervision;

import android.content.Intent;
import android.database.Cursor;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.net.Uri;
import android.provider.MediaStore;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.Log;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.widget.ImageView;
import android.widget.TextView;

import com.ganesus.numbervision.engine.ChainCodeGenerator;
import com.ganesus.numbervision.engine.Interpretator;
import com.ganesus.numbervision.engine.NativeBitmap;
import com.ganesus.numbervision.engine.ToNxN;
import com.ganesus.numbervision.engine.TurnCodeImpl;
import com.ganesus.numbervision.engine.ZhangSuenGenerator;

import java.util.List;

public class Vision5 extends AppCompatActivity {
    private static final int RESULT_LOAD_IMAGE = 1212;
    private Interpretator interpretator;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_vision5);

        interpretator = new Interpretator(getResources(), R.raw.tipis);
    }


    public void klikGallery(View v){
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

            if (bmp.getWidth() == 1632){
                bmp = Bitmap.createBitmap(bmp, 546, 491, 549, 115);
            }else if(bmp.getWidth() == 1024){
                bmp = Bitmap.createBitmap(bmp, 345, 323, 390, 107);
            }else if(bmp.getWidth() == 1019){
                bmp = Bitmap.createBitmap(bmp, 341, 438, 332, 55);
            }

            NativeBitmap nativeBitmap = new NativeBitmap(bmp);
            nativeBitmap.smooth();
            nativeBitmap.grayscaleBitmap();

            int w = bmp.getWidth(); int h = bmp.getHeight();
            bmp.recycle();

            boolean[][] boolImage = nativeBitmap.convertToBoolmage();
            boolean[][] boolImage2 = nativeBitmap.convertToBoolmage();

            Bitmap hasil = nativeBitmap.draw(boolImage);

            ZhangSuenGenerator zhangSuenGenerator = new ZhangSuenGenerator();
            zhangSuenGenerator.doZhangSuenThinning(boolImage,true);

            ImageView iv = (ImageView) findViewById(R.id.imageView);
            iv.setImageBitmap(nativeBitmap.draw(boolImage));

            ChainCodeGenerator ccg = new ChainCodeGenerator();
            List<ChainCodeGenerator.BorderInfo> borderInfos = ccg.sorter(ccg.getBorderInfos(boolImage,w,h));

            ToNxN compressor = new ToNxN(5);
            StringBuilder outText = new StringBuilder();

            for (int i = 0 ; i < borderInfos.size(); i++){
                if (borderInfos.get(i).chainCodes.length() > 0) {
                    Log.d("DEBUG",borderInfos.get(i).chainCodes);

                    outText.append(interpretator.guessChain(borderInfos.get(i).chainCodes));
                }
            }

            TextView txtBelok = (TextView) findViewById(R.id.txtInterpretasi);
            txtBelok.setText(outText.toString());


            Log.d("DEBUG", "Sudah selesai");
        }
    }
}
