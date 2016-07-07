package com.liekkas;

import android.content.res.AssetManager;

public class Liekkas {
    // for engine
    public static native boolean engineInit(AssetManager mgr);
    public static native void engineDestory();

    // static {
    //     System.loadLibrary("Liekkas");
    // }
}