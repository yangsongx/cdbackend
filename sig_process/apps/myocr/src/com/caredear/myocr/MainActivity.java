package com.caredear.myocr;

import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;

import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;
import javax.xml.parsers.ParserConfigurationException;
import javax.xml.xpath.XPath;
import javax.xml.xpath.XPathConstants;
import javax.xml.xpath.XPathExpressionException;
import javax.xml.xpath.XPathFactory;

import org.w3c.dom.NodeList;
import org.xml.sax.SAXException;

import android.app.Activity;
import android.os.Bundle;
import android.provider.DocumentsContract.Document;
import android.view.Menu;

import com.googlecode.tesseract.android.TessBaseAPI;

public class MainActivity extends Activity {
	public static final int OCR_ENG = 0;
	public static final int OCR_CH = 1;
	public static final int OCR_TW = 2;
	
	private TessBaseAPI baseApi;
	// FIXME maybe we need made this configurable
	private int ocrEngineMode = TessBaseAPI.OEM_TESSERACT_ONLY;
	
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_main);
		
		String ret = OcrUtil.getImg("/data/data/tf.png");
		
		android.util.Log.e("a22301","@C++ tell me:" + ret);
		if(initOcrEngine(OCR_CH) == 0)
		{
			android.util.Log.e("a22301", "Cool to init the OCR");
		}
	}

	@Override
	public boolean onCreateOptionsMenu(Menu menu) {
		// Inflate the menu; this adds items to the action bar if it is present.
		getMenuInflater().inflate(R.menu.main, menu);
		return true;
	}
	
	/**
	 * Init the OCR lib engine
	 * @return
	 */
	private int initOcrEngine(int type){
		baseApi = new TessBaseAPI();
		String lang;
		if(type == OCR_CH){
			lang = "chi_sim";
		} else {
			lang = "chi_sim";
		}
		
		if(baseApi.init("/mnt/sdcard/ocrdata/",
				"chi_sim",
				0)) {
			android.util.Log.e("a22301", "cool, init OK");
			//FIXME - maybe we can set as one-single line mode
			baseApi.setPageSegMode(TessBaseAPI.PageSegMode.PSM_AUTO_OSD);
			android.util.Log.e("a22301", "checking txt...");
			File file = new File("/data/data/test.png");
			if(file.exists()){
				baseApi.setImage(file);
				String txt = baseApi.getUTF8Text();
				android.util.Log.e("a22301", "txt:" + txt);	
			}
			
		}
		else{
			android.util.Log.e("a22301", "failed init OCR data dir");
		}
		return 0;
	}
	
	private void checkLayoutXml(String xmlFile){
		DocumentBuilderFactory factory = DocumentBuilderFactory.newInstance();
		factory.setNamespaceAware(true);
		try {
			DocumentBuilder builder = factory.newDocumentBuilder();
			org.w3c.dom.Document doc = builder.parse(new FileInputStream(new File(xmlFile)));
			
			XPathFactory xf = XPathFactory.newInstance();
			XPath  path = xf.newXPath();
			//String result = path.evaluate("/config/user/name",doc);
			//android.util.Log.e("a22301", "result:" + result);
			
			
			NodeList nodes = (NodeList)path.evaluate("/config/", doc, XPathConstants.NODESET);
			
			//nodes.
			android.util.Log.e("a22301", "Totally " + nodes.getLength() + " nodes");
		} catch (ParserConfigurationException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		} catch (IOException e){
			
		} 
		catch (SAXException e){}
		catch (XPathExpressionException e){}
	}

}
