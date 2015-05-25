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
	public native void stopServer();
	
	//public native String startClient(int port); // return value device info(including IP)
	public native String fetchTargetDev(int port); // this is an async call type.
	public native void stopClient();
	
	
	EditText  mEditPort;
	ToggleButton mToggleBtn;
	CheckBox mChkBox;
	
	private static final String TAG = "PEERDETECT";
	
	int mRunningMode = 0; // 0 - server , 1 -client
	
	OnCheckedChangeListener mListener = new OnCheckedChangeListener(){
		public void onCheckedChanged(CompoundButton buttonView, boolean isChecked){
			android.util.Log.e(TAG, "running mode = " + mRunningMode);
			if(isChecked){
				// User want to start
				String val = mEditPort.getText().toString();
				if(mRunningMode == 0){
				    
				    try_launch_the_server(0); // FIXME currenlty, parameter not used...
				    
				} else {
				    /* Client */
				    try_get_tareget_dev_list();
				    /*
					String ip = startClient(Integer.parseInt(val));
					
					AlertDialog.Builder builder = new Builder(MainActivity.this);
					builder.setMessage(ip);
					builder.setTitle("IP Address");
					builder.create().show();
					*/
				}
				
			} else {
				// User want to stop
			    if(mRunningMode == 0){
			        //Stop the server!
			        android.util.Log.e(TAG, "Stop broadcast server!");
			        stopServer();
			    } else {
			        android.util.Log.e(TAG, "Stop client's detecting code, FAKE code");
			        //stopClient();
			    }
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
	
	private void try_launch_the_server(int port) {
	    new Thread(new Runnable() {

            @Override
            public void run() {
                // TODO Auto-generated method stub
                String val = mEditPort.getText().toString();
                android.util.Log.e(TAG, "Java-layer- drop to C++(port:" + val + ")");
                startServer(Integer.parseInt(val));
            }
	        
	    }).start();
	}
	
	private void try_get_tareget_dev_list() {
	    String val = mEditPort.getText().toString();
	    android.util.Log.e(TAG, "Java-layer- drop to C++ to get dev list...(port:" + val + ")");
	    String result = fetchTargetDev(Integer.parseInt(val));
        
        AlertDialog.Builder builder = new Builder(MainActivity.this);
        builder.setMessage(result);
        builder.setTitle("Scan Result");
        builder.create().show();
	}
}
