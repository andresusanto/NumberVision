    package com.ganesus.numbervision;

import android.content.Intent;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.SeekBar;
import android.widget.TextView;

import java.nio.ByteBuffer;

    
public class EqualigramMain extends AppCompatActivity {
    private ByteBuffer buffBitmap = null;
    private ByteBuffer buffBitmapGray = null;
    private ByteBuffer buffBitmap1 = null;

    private SeekBar minSeekbar, maxSeekbar;
    private TextView minTextView, maxTextView;
    private int min_rgb_value, max_rgb_value;


    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_equaligram_main);

        Intent intent = getIntent();

        Bitmap bmp = BitmapFactory.decodeFile(intent.getStringExtra("PATH"));
        if (bmp.getWidth() > 2000){
            bmp = Bitmap.createScaledBitmap(bmp, bmp.getWidth()/2, bmp.getHeight()/2, false);
        }

        buffBitmap = loadBitmap(bmp);
        buffBitmapGray = createGrayscale(buffBitmap);

        Bitmap.Config conf = Bitmap.Config.ARGB_8888;
        Bitmap canvas = Bitmap.createBitmap(780, 320, conf);


        ImageView ivAsli = (ImageView) findViewById(R.id.ivAsli);
        ImageView hisAsli = (ImageView) findViewById(R.id.hisAsli);
        ImageView hisGray = (ImageView) findViewById(R.id.hisGray);
        ImageView ivGray = (ImageView) findViewById(R.id.ivGray);
        final ImageView hisAlgo1 = (ImageView) findViewById(R.id.hisAlgo1);
        final ImageView ivAlgo1= (ImageView) findViewById(R.id.ivAlgo1);

        TextView tvAlgoName = (TextView) findViewById(R.id.txtAlgoName);
        TextView tvAlgoHis = (TextView) findViewById(R.id.txtAlgoHis);

        LinearLayout layoutMax = (LinearLayout) findViewById(R.id.layoutSliderMax);
        LinearLayout layoutMin = (LinearLayout) findViewById(R.id.layoutSliderMin);


        switch (intent.getIntExtra("FUNCTION", R.id.rCummulative)){
            case R.id.rCummulative:
                buffBitmap1 = applyAlgo1(buffBitmapGray);
                ivAlgo1.setImageBitmap(applyAlgo1Bmp(buffBitmapGray));
                tvAlgoName.setText("Hasil Algoritma Kumulatif");
                tvAlgoHis.setText("Histogram Algoritma Kumulatif");
                layoutMax.setVisibility(View.GONE);
                layoutMin.setVisibility(View.GONE);
                break;
            case R.id.rSimple:
                buffBitmap1 = applyAlgo2(buffBitmapGray);
                ivAlgo1.setImageBitmap(applyAlgo2Bmp(buffBitmapGray));
                tvAlgoName.setText("Hasil Algoritma Simpel");
                tvAlgoHis.setText("Histogram Algoritma Simpel");
                layoutMax.setVisibility(View.GONE);
                layoutMin.setVisibility(View.GONE);
                break;
            case R.id.rLine:
                this.minSeekbar = (SeekBar) findViewById(R.id.minSeekbar);
                this.maxSeekbar = (SeekBar) findViewById(R.id.maxSeekbar);

                this.minTextView = (TextView) findViewById(R.id.minTextView);
                this.maxTextView = (TextView) findViewById(R.id.maxTextView);

                min_rgb_value = Integer.parseInt(minTextView.getText().toString()); minTextView.setText(String.valueOf(min_rgb_value));
                max_rgb_value = Integer.parseInt(maxTextView.getText().toString()); maxTextView.setText(String.valueOf(max_rgb_value));

                tvAlgoName.setText("Hasil Algoritma Linear");
                tvAlgoHis.setText("Histogram Algoritma Linear");

                buffBitmap1 = applyAlgoLinear(buffBitmapGray,min_rgb_value,max_rgb_value);
                ivAlgo1.setImageBitmap(applyAlgoLinearBmp(buffBitmapGray, min_rgb_value, max_rgb_value));

                this.minSeekbar.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
                    @Override
                    public void onProgressChanged(SeekBar seekBar, int progress, boolean b) {
                        minTextView.setText(String.valueOf(progress));
                        min_rgb_value = progress;

                        Bitmap.Config conf = Bitmap.Config.ARGB_8888;
                        Bitmap canvas = Bitmap.createBitmap(520, 320, conf);

                        buffBitmap1 = applyAlgoLinear(buffBitmapGray,min_rgb_value,max_rgb_value);
                        ivAlgo1.setImageBitmap(applyAlgoLinearBmp(buffBitmapGray,min_rgb_value,max_rgb_value));
                        hisAlgo1.setImageBitmap(genHistogram(buffBitmap1, canvas, true));
                    }

                    @Override
                    public void onStartTrackingTouch(SeekBar seekBar) {
                    }

                    @Override
                    public void onStopTrackingTouch(SeekBar seekBar) {
                    }
                });

                this.maxSeekbar.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
                    @Override
                    public void onProgressChanged(SeekBar seekBar, int progress, boolean b) {
                        maxTextView.setText(String.valueOf(progress));
                        max_rgb_value = progress;

                        Bitmap.Config conf = Bitmap.Config.ARGB_8888;
                        Bitmap canvas = Bitmap.createBitmap(520, 320, conf);

                        buffBitmap1 = applyAlgoLinear(buffBitmapGray,min_rgb_value,max_rgb_value);
                        ivAlgo1.setImageBitmap(applyAlgoLinearBmp(buffBitmapGray,min_rgb_value,max_rgb_value));
                        hisAlgo1.setImageBitmap(genHistogram(buffBitmap1, canvas, true));
                    }

                    @Override
                    public void onStartTrackingTouch(SeekBar seekBar) {
                    }

                    @Override
                    public void onStopTrackingTouch(SeekBar seekBar) {
                    }
                });
                break;

            case R.id.rStep:
                buffBitmap1 = applyAlgoStep(buffBitmapGray, 0);
                ivAlgo1.setImageBitmap(applyAlgoStepBmp(buffBitmapGray, 0));
                tvAlgoName.setText("Hasil Algoritma Step");
                tvAlgoHis.setText("Histogram Algoritma Step");
                layoutMax.setVisibility(View.GONE);

                this.minSeekbar = (SeekBar) findViewById(R.id.minSeekbar);
                this.minTextView = (TextView) findViewById(R.id.minTextView);

                min_rgb_value = Integer.parseInt(minTextView.getText().toString()); minTextView.setText(String.valueOf(min_rgb_value));

                this.minSeekbar.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
                    @Override
                    public void onProgressChanged(SeekBar seekBar, int progress, boolean b) {
                        minTextView.setText(String.valueOf(progress));
                        min_rgb_value = progress;

                        Bitmap.Config conf = Bitmap.Config.ARGB_8888;
                        Bitmap canvas = Bitmap.createBitmap(520, 320, conf);

                        buffBitmap1 = applyAlgoStep(buffBitmapGray, min_rgb_value);
                        ivAlgo1.setImageBitmap(applyAlgoStepBmp(buffBitmapGray, min_rgb_value));
                        hisAlgo1.setImageBitmap(genHistogram(buffBitmap1, canvas, true));
                    }

                    @Override
                    public void onStartTrackingTouch(SeekBar seekBar) {
                    }

                    @Override
                    public void onStopTrackingTouch(SeekBar seekBar) {
                    }
                });
                break;
        }

        ivAsli.setImageBitmap(bmp);
        hisAsli.setImageBitmap(genHistogram(buffBitmap, canvas, false));
        ivGray.setImageBitmap(createGrayscaleBmp(buffBitmap));

        canvas = Bitmap.createBitmap(520, 320, conf);
        hisGray.setImageBitmap(genHistogram(buffBitmapGray, canvas, true));

        canvas = Bitmap.createBitmap(520, 320, conf);
        hisAlgo1.setImageBitmap(genHistogram(buffBitmap1, canvas, true));

    }

    public native ByteBuffer loadBitmap(Bitmap bitmap);
    public native ByteBuffer createGrayscale(ByteBuffer bitmem);
    public native ByteBuffer applyAlgo1(ByteBuffer bitmem);
    public native ByteBuffer applyAlgo2(ByteBuffer bitmem);
    public native ByteBuffer applyAlgoLinear(ByteBuffer bitmem,int new_min,int new_max);
    public native ByteBuffer applyAlgoStep(ByteBuffer bitmem,int L);

    public native Bitmap genHistogram(ByteBuffer bitmem, Bitmap canvas, boolean isGrayScale);
    public native Bitmap createGrayscaleBmp(ByteBuffer bitmem);
    public native Bitmap applyAlgo1Bmp(ByteBuffer bitmem);
    public native Bitmap applyAlgo2Bmp(ByteBuffer bitmem);
    public native Bitmap applyAlgoLinearBmp(ByteBuffer bitmem,int new_min,int new_max);
    public native Bitmap applyAlgoStepBmp(ByteBuffer bitmem,int L);

    static {
        System.loadLibrary("equaligram");
    }
}
