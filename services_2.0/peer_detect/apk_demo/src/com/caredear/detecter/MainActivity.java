package com.caredear.detecter;

import android.os.Bundle;
import android.app.Activity;
import android.app.AlertDialog;
import android.app.AlertDialog.Builder;
import android.view.Menu;
import android.widget.CheckBox;
import android.widget.CompoundButton;
import android.widget.CompoundButton.OnCheckedChangeListener;
import android.widget.EditText;
import android.widget.ToggleButton;

public class MainActivity extends Activity {
	
	static {
		System.loadLibrary("peerdetect");
	}

	public native void startServer(int port);
	public native String startClient(int port); // return value is the target IP
	
	EditText  mEditPort;
	ToggleButton mToggleBtn;
	CheckBox mChkBox;
	
	int mRunningMode = 0; // 0 - server , 1 -client
	
	OnCheckedChangeListener mListener = new OnCheckedChangeListener(){
		public void onCheckedChanged(CompoundButton buttonView, boolean isChecked){
			android.util.Log.e("a22301", "running mode = " + mRunningMode);
			if(isChecked){
				// User want to start
				String val = mEditPort.getText().toString();
				if(mRunningMode == 0){
					startServer(Integer.parseInt(val));
				} else {
					String ip = startClient(Integer.parseInt(val));
					
					AlertDialog.Builder builder = new Builder(MainActivity.this);
					builder.setMessage(ip);
					builder.setTitle("IP Address");
					builder.create().show();
				}
				
			} else {
				// User want to stop

			}
		}
	};
	
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_main);
		
		mEditPort = (EditText)findViewById(R.id.editText1);
		mEditPort.setText("2121"); // default port is 2121
		
		mToggleBtn = (ToggleButton)findViewById(R.id.toggleStartStopBtn);
		mToggleBtn.setOnCheckedChangeListener(mListener);

		mChkBox = (CheckBox)findViewById(R.id.checkBox1);
		mChkBox.setOnCheckedChangeListener(new OnCheckedChangeListener(){

			@Override
			public void onCheckedChanged(CompoundButton arg0, boolean arg1) {
				// TODO Auto-generated method stub
				if(arg1){
					mRunningMode = 0;
				} else {
					mRunningMode = 1;
				}
			}
			
		});
		if(mRunningMode == 0){
			mChkBox.setChecked(true);
		} else {
			mChkBox.setChecked(false);
		}
	}

	@Override
	public boolean onCreateOptionsMenu(Menu menu) {
		// Inflate the menu; this adds items to the action bar if it is present.
		getMenuInflater().inflate(R.menu.main, menu);
		return true;
	}
}
