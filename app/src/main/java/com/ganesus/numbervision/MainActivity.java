package com.ganesus.numbervision;

import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;

public class MainActivity extends AppCompatActivity {

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        detectAll(BitmapFactory.decodeResource(getResources(), R.drawable.learning));

    }


    public native String[] detectAll(Bitmap bitmap);

    static {
        System.loadLibrary("numbervision");
    }
}
