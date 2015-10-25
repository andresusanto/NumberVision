package com.ganesus.numbervision;

import android.content.Intent;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;

public class Selector extends AppCompatActivity {

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_selector);
    }

    public void clickSelect(View v){
        Intent i;
        switch (v.getId()){
            case R.id.button2 :
                i = new Intent(this, DetectWarnaActivity.class);
                break;
            case R.id.button3 :
                i = new Intent(this, EqualigramSelector.class);
                break;
            case R.id.button4 :
                i = new Intent(this, Vision1.class);
                break;
            case R.id.button5 :
                i = new Intent(this, Vision2.class);
                break;
            case R.id.button6 :
                i = new Intent(this, Vision3.class);
                break;
            case R.id.button7 :
                i = new Intent(this, DetectWarnaActivity.class);
                break;
            default :
                i = new Intent(this, DetectWarnaActivity.class);
                break;

        }
        startActivity(i);
    }

}
