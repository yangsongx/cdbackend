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
	
	/**
	 * Try to match the text on @imgName file
	 * @param imgName
	 * @return
	 */
	public static native String matchingTextOnImg(String imgName);
	
	/**
	 * Handling the image withing specified rectangle box
	 * @param imgName
	 * @param x1
	 * @param y1
	 * @param x2
	 * @param y2
	 * @return
	 */
	public static native String processTargetImg(String imgName, int x1, int y1, int x2, int y2);
}
