package com.ganesus.numbervision;

import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.view.Menu;
import android.view.MenuItem;
import android.widget.TextView;

public class MainActivity extends AppCompatActivity {

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        TextView tv = (TextView) findViewById(R.id.textView);

        String tes[] = detectAll(BitmapFactory.decodeResource(getResources(), R.drawable.tc2));

        tv.setText(tes[0]);
    }


    public native String[] detectAll(Bitmap bitmap);

    static {
        System.loadLibrary("numbervision");
    }
}
