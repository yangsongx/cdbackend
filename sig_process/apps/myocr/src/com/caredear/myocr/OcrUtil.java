package com.caredear.myocr;

/**
 * Aim to be a C++/Java JNI API wrapper
 * 
 * @author yang
 *
 */
public class OcrUtil {
	
	static {
		System.loadLibrary("myocr");
	}

	public static native String getImg(String fn);
	
}
