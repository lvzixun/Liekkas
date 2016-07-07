package com.example.testaudio;

import com.liekkas.Liekkas;
import android.app.Activity;
import android.content.res.AssetManager;
import android.os.Bundle;



public class TestAudio extends Activity {
    static AssetManager assetManager;

    @Override
    protected void onCreate(Bundle icicle) {
        super.onCreate(icicle);
        assetManager = getAssets();

        Liekkas.engineInit(assetManager);
        testInit();
    }

    @Override
    protected void onPause() {
        super.onPause();
    }

    @Override
    protected void onDestroy() {
        testDestory();
        Liekkas.engineDestory();
        
        super.onDestroy();
    }

    // for test
    public static native void testInit();
    public static native void testDestory();


    static {
        System.loadLibrary("Liekkas");
    }
}