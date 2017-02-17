package com.lenovo.smartengine.activity;
/*
 * SmartSingleActivity.java
 *
 *	Main activity of smart app,
 *  Add comments on variable and functions, clear up the unused part
 *
 *  Revised on: 2016/04/08
 *      Author: fengbin1
 */
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.util.List;

import android.annotation.TargetApi;
import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.content.pm.ActivityInfo;
import android.content.res.Configuration;
import android.graphics.SurfaceTexture;
import android.hardware.Camera;
import android.hardware.Sensor;
import android.hardware.SensorEvent;
import android.hardware.SensorEventListener;
import android.hardware.SensorManager;
import android.hardware.Camera.AutoFocusCallback;
import android.opengl.GLSurfaceView;
import android.os.Build;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.util.DisplayMetrics;
import android.util.Log;
import android.view.Display;
import android.view.GestureDetector;
import android.view.GestureDetector.OnGestureListener;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.view.MotionEvent;
import android.view.OrientationEventListener;
import android.view.View;
import android.view.ViewConfiguration;
import android.view.Window;
import android.view.WindowManager;
import android.widget.Button;
import android.widget.FrameLayout;
import android.widget.ImageButton;
import android.widget.LinearLayout;
import android.widget.RatingBar;
import android.widget.TextView;
import android.widget.Toast;

import com.example.arframetest.R;
import com.lenovo.smartengine.gl.model.GLSmartPlugin;
import com.lenovo.smartengine.jni.SmartNative;
import com.lenovo.smartengine.util.GlobalParams;
import com.lenovo.smartengine.view.CameraSurfaceView;
import com.lenovo.smartengine.view.SmartSceneInfoView;
import com.lenovo.smartengine.view.renderer.SmartSingleRenderer;

public class SmartSingleActivity extends Activity implements
		SurfaceTexture.OnFrameAvailableListener, OnGestureListener {

	private static final String TAG = "SmartSingleActivity";

	public static final int ORIENTATION_HYSTERESIS = 5;
	public static final int MSG_UPDATE_UI = 0x01;

	long index = 0;

	//sensor management object(accelerometer|orientation|light)
	private SensorManager sensorMgr;
	private OrientationListener oriListener;
	private int orientationRaw = 0;

	//show the detection information on another view
	static SmartSceneInfoView smartSceneInfoView;// for display scene result information
	
	static LinearLayout foodhealthlayout;
	static FrameLayout mylay;
	static TextView textView1;
	static RatingBar ratingBar2;
	ImageButton foodbutton;
	private Button icgbutton;
	private Button tldbutton;
	public static FrameLayout foodfoglay;
	private GLSurfaceView glView;
	// private CardboardView glView;
	private CameraSurfaceView camSurface;

	private FrameLayout mainLayout;
	GestureDetector detector;

	private FrameLayout face_layout;
	private static LinearLayout face;
	private static TextView age;
	private static TextView faceValue;
	private ImageButton facebutton;
	private static long firstTime;
	public static boolean focusFlag = false;
	static {
		if (GlobalParams.OPEN_FACE) {
			System.loadLibrary("ope ncv_java");
			System.loadLibrary("CRBCamera");
		}
	}
   
	/** Called when the activity is first created. */
	@TargetApi(Build.VERSION_CODES.HONEYCOMB)
	@Override
	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		GlobalParams.INIT_ACTIVITY = 0;
		detector = new GestureDetector(this, this);
		boolean needActionBar = false;
		if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.HONEYCOMB) {
			if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.ICE_CREAM_SANDWICH) {
				if (!ViewConfiguration.get(this).hasPermanentMenuKey())
					needActionBar = true;
			} else {
				needActionBar = true;
			}
		}
		if (needActionBar) {
			requestWindowFeature(Window.FEATURE_ACTION_BAR);
		} else {
			requestWindowFeature(Window.FEATURE_NO_TITLE);
			getWindow().addFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN);
		}

		setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE); // Force
																			// landscape-only.
		updateNativeDisplayParameters();

		setContentView(R.layout.main);

		initManager(this);
		mainLayout = (FrameLayout) this.findViewById(R.id.mainLayout);
		foodfoglay = (FrameLayout) findViewById(R.id.food_fog_lay);
		mylay = (FrameLayout) findViewById(R.id.frameLayout1);
		textView1 = (TextView) findViewById(R.id.textView1);
		ratingBar2 = (RatingBar) findViewById(R.id.ratingBar2);
		foodbutton = (ImageButton) findViewById(R.id.foodbutton);
		foodhealthlayout = (LinearLayout) findViewById(R.id.food);
		// face
		face_layout = (FrameLayout) findViewById(R.id.face_frameLayout);
		face = (LinearLayout) findViewById(R.id.face);
		age = (TextView) findViewById(R.id.age);
		faceValue = (TextView) findViewById(R.id.faceValue);
		facebutton = (ImageButton) findViewById(R.id.facebutton);
		icgbutton = (Button)findViewById(R.id.button_icg);	
		icgbutton.getBackground().setAlpha(127);
		
		tldbutton = (Button)findViewById(R.id.button_tld);
		tldbutton.getBackground().setAlpha(127);
		
		smartSceneInfoView = (SmartSceneInfoView) findViewById(R.id.smart_scene_info_view);
		
	}

	@Override
	public void onStart() {
		super.onStart();
		GlobalParams.INIT_ACTIVITY = 0;
	}

	@SuppressWarnings("deprecation")
	// FILL_PARENT still required for API level 7 (Android 2.1)
	@Override
	public void onResume() {
		super.onResume();
		GlobalParams.INIT_ACTIVITY = 0;
		// Create the camera view.
		camSurface = new CameraSurfaceView(this);
	

		// Create/recreate the GL view.
		// glView = new GL2SurfaceView(this, true, false, 16, 0);
		glView = new GLSurfaceView(this);
		glView.setEGLContextClientVersion(2);// add by mikey
		glView.setRenderer(new SmartSingleRenderer(this));

		mainLayout.addView(camSurface, 0);
		mainLayout.addView(glView, 0);
		glView.setZOrderMediaOverlay(true);

		registerSensor();
		if (glView != null)
			glView.onResume();
	}

	@Override
	protected void onPause() {
		super.onPause();
		if (glView != null)
			glView.onPause();

		mainLayout.removeView(glView);
		mainLayout.removeView(camSurface);
		unregisterSensor();
	}

	@Override
	public void onStop() {
		super.onStop();

		unregisterSensor();
	}

	@Override
	public void onDestroy() {
		super.onDestroy();
	}

	@Override
	public void onConfigurationChanged(Configuration newConfig) {
		super.onConfigurationChanged(newConfig);

		updateNativeDisplayParameters();
	}
	
	@Override
	public void onFrameAvailable(SurfaceTexture surfaceTexture) {
		// TODO Auto-generated method stub
		glView.requestRender();
	}
	
	@Override
	public boolean onTouchEvent(MotionEvent event) {
		int modelFlag = SmartSingleRenderer.foodModelFlag;
		detector.onTouchEvent(event);
		Camera.Parameters parameters = CameraSurfaceView.camera.getParameters();
		//List<Camera.Size> previewsize = parameters.getSupportedPreviewSizes();
		
		if(CameraSurfaceView.track_roi[0] == GlobalParams.TRACKING_STATUS_INIT)
			return true;
		
		switch (event.getAction()) {

		case MotionEvent.ACTION_DOWN:
			parameters.setFocusMode(Camera.Parameters.FOCUS_MODE_AUTO);
			CameraSurfaceView.camera.setParameters(parameters);
			CameraSurfaceView.camera.autoFocus(autoFoucsCallback);
//			Log.i("FoucusFlag", "Touch Down Successfully");
//			GLSmartPlugin.actionEventDownTure = true;
//			GLSmartPlugin.actionEventDown_x =  event.getX();
//			GLSmartPlugin.actionEventDown_y =  event.getY();
			
			CameraSurfaceView.track_roi[0] = GlobalParams.TRACKING_STATUS_IDLE;
			CameraSurfaceView.track_roi[1] = (int) event.getX();
			CameraSurfaceView.track_roi[2] = (int) event.getY();
			CameraSurfaceView.track_roi[3] = (int) event.getX();
			CameraSurfaceView.track_roi[4] = (int) event.getY();
			break;
		case MotionEvent.ACTION_MOVE:
			
			CameraSurfaceView.track_roi[0] = GlobalParams.TRACKING_STATUS_MOVE;
			CameraSurfaceView.track_roi[3] = (int) event.getX();
			CameraSurfaceView.track_roi[4] = (int) event.getY();
			break;
		case MotionEvent.ACTION_UP:
			CameraSurfaceView.track_roi[0] = GlobalParams.TRACKING_STATUS_INIT;
			CameraSurfaceView.track_roi[3] = (int) event.getX();
			CameraSurfaceView.track_roi[4] = (int) event.getY();
		default:
			break;
		}

		return false;
	}
	//set auto focus callback
	AutoFocusCallback autoFoucsCallback = new AutoFocusCallback(){

		@Override
		public void onAutoFocus(boolean success, Camera camera) {
			// TODO Auto-generated method stub
			
		}
		
	};
	/**
	 * get screen size of phone
	 */
	private void updateNativeDisplayParameters() {
		Display display = getWindowManager().getDefaultDisplay();
		int orientation = display.getRotation();
		DisplayMetrics outMetrics = new DisplayMetrics();
		display.getMetrics(outMetrics);
		int w = outMetrics.widthPixels;
		int h = outMetrics.heightPixels;
		int dpi = outMetrics.densityDpi;
		GlobalParams.SCREEN_WIDTH = w;
		GlobalParams.SCREEN_HEIGHT = h;
	}
	
	/**
	 * initial sensor listener
	 * @param context  activity context
	 */
	public void initManager(Context context) {
		sensorMgr = (SensorManager) context.getSystemService(Context.SENSOR_SERVICE);
		oriListener = new OrientationListener(context);
	}

	/**
	 * register sensor
	 */
	private void registerSensor() {
		sensorMgr.registerListener(sensorListener,
				sensorMgr.getDefaultSensor(Sensor.TYPE_ACCELEROMETER),
				SensorManager.SENSOR_DELAY_NORMAL);
		sensorMgr.registerListener(sensorListener,
				sensorMgr.getDefaultSensor(Sensor.TYPE_LIGHT),
				SensorManager.SENSOR_DELAY_UI);
		sensorMgr.registerListener(sensorListener,
				sensorMgr.getDefaultSensor(Sensor.TYPE_ORIENTATION),
				SensorManager.SENSOR_DELAY_NORMAL);
		oriListener.enable();
	}

	/**
	 * unregister sensor
	 */
	private void unregisterSensor() {
		sensorMgr.unregisterListener(sensorListener);
		oriListener.disable();
	}

	private SensorEventListener sensorListener = new SensorEventListener() {
		float[] aSensor = new float[3];
		float[] oSensor = new float[3];
		float[] lSensor = new float[3];

		@Override
		public void onSensorChanged(SensorEvent e) {
			int type = e.sensor.getType();
			switch (type) {
			case Sensor.TYPE_ACCELEROMETER:
				aSensor[0] = e.values[0];
				aSensor[1] = e.values[1];
				aSensor[2] = e.values[2];
				SmartNative.setAsensorData(aSensor, aSensor.length);
				break;
			case Sensor.TYPE_ORIENTATION:
				oSensor[0] = e.values[0];
				oSensor[1] = e.values[1];
				oSensor[2] = e.values[2];
				SmartNative.setOsensorData(oSensor, oSensor.length);
				break;
			case Sensor.TYPE_LIGHT:
				lSensor[0] = e.values[0];
				lSensor[1] = e.values[1];
				lSensor[2] = e.values[2];
				SmartNative.setLsensorData(lSensor, 1);
				break;
			}
		}

		@Override
		public void onAccuracyChanged(Sensor arg0, int arg1) {
		}
	};

	private class OrientationListener extends OrientationEventListener {
		public OrientationListener(Context context) {
			super(context);
		}

		@Override
		public void onOrientationChanged(int ori) {
			if (ori == ORIENTATION_UNKNOWN)
				return;
			orientationRaw = ori;
			GlobalParams.orientation = roundOrientation(ori, GlobalParams.orientation);
			SmartNative.setOrientation(orientationRaw);
//			SmartNative.setOrientation(GlobalParams.orientation);
		}
	}
	
	/**
	 * split 360 angle into [0, 90, 180, 270]
	 * @param orientation
	 * @param orientationHistory
	 * @return
	 */
	private int roundOrientation(int orientation, int orientationHistory) {
		boolean changeOrientation = false;
		if (orientationHistory == OrientationEventListener.ORIENTATION_UNKNOWN) {
			changeOrientation = true;
		} else {
			int dist = Math.abs(orientation - orientationHistory);
			dist = Math.min(dist, 360 - dist);
			changeOrientation = (dist >= 45 + ORIENTATION_HYSTERESIS);
		}
		if (changeOrientation) {
			return ((orientation + 45) / 90 * 90) % 360;
		}
		return orientationHistory;
	}
	
	@Override
	public boolean onDown(MotionEvent e) {
		// TODO Auto-generated method stub
		return false;
	}

	@Override
	public void onShowPress(MotionEvent e) {
		// TODO Auto-generated method stub

	}

	@Override
	public boolean onSingleTapUp(MotionEvent e) {
		// TODO Auto-generated method stub
		return false;
	}

	@Override
	public boolean onScroll(MotionEvent e1, MotionEvent e2, float distanceX,
			float distanceY) {
		// TODO Auto-generated method stub
		return false;
	}

	@Override
	public void onLongPress(MotionEvent e) {
		// TODO Auto-generated method stub

	}

	@Override
	public boolean onFling(MotionEvent e1, MotionEvent e2, float velocityX, float velocityY) {
		// TODO Auto-generated method stub
		//fling on the screen to contral different AR boject
		if(CameraSurfaceView.sceneResult[0] == 8)
		{
		if (e1.getX() - e2.getX() < 50) {
			SmartSingleRenderer.foodModelFlag++;
			if (SmartSingleRenderer.foodModelFlag > SmartSingleRenderer.FOOD_MODLE_NUMBERS - 1)
				SmartSingleRenderer.foodModelFlag = SmartSingleRenderer.FOOD_MODLE_NUMBERS - 1;
			SmartSingleRenderer.foodModelFlag = SmartSingleRenderer.foodModelFlag % SmartSingleRenderer.FOOD_MODLE_NUMBERS;
		} else if (e1.getX() - e2.getX() > 50) {
			SmartSingleRenderer.foodModelFlag--;
			if (SmartSingleRenderer.foodModelFlag < 0)
				SmartSingleRenderer.foodModelFlag = 0;
			SmartSingleRenderer.foodModelFlag = SmartSingleRenderer.foodModelFlag % SmartSingleRenderer.FOOD_MODLE_NUMBERS;
		}
		}else if(CameraSurfaceView.sceneResult[0] == 9)
		{
			if (e1.getX() - e2.getX() < 50) {
				SmartSingleRenderer.qrcodeModelFlag++;
				if (SmartSingleRenderer.qrcodeModelFlag > SmartSingleRenderer.QRCODE_MODLE_NUMBERS - 1)
					SmartSingleRenderer.qrcodeModelFlag = SmartSingleRenderer.QRCODE_MODLE_NUMBERS - 1;
				SmartSingleRenderer.qrcodeModelFlag = SmartSingleRenderer.qrcodeModelFlag % SmartSingleRenderer.QRCODE_MODLE_NUMBERS;
			} else if (e1.getX() - e2.getX() > 50) {
				SmartSingleRenderer.qrcodeModelFlag--;
				if (SmartSingleRenderer.qrcodeModelFlag < 0)
					SmartSingleRenderer.qrcodeModelFlag = 0;
				SmartSingleRenderer.qrcodeModelFlag = SmartSingleRenderer.qrcodeModelFlag % SmartSingleRenderer.QRCODE_MODLE_NUMBERS;
			}
		}
		return true;
	}

	/**
	 * show the food attractiveness score
	 */
	private static void food_health_display() {
		Log.i("TouchLog", "Touch: ");
		if (CameraSurfaceView.sceneResult[0] == 8) {
			switch (SmartSingleRenderer.foodModelFlag) {
			case SmartSingleRenderer.FOOD_DISPLAY_FOG:
//				foodfoglay
//						.setLeft((int) SmartNative.getCompositeResult()[5] - 100);
//				foodfoglay
//						.setTop(GlobalParams.SCREEN_HEIGHT - (int) SmartNative.getCompositeResult()[4] - 300);
//				foodfoglay.setVisibility(View.VISIBLE);
				foodhealthlayout.setVisibility(View.INVISIBLE);
				break;
			case SmartSingleRenderer.FOOD_DISPLAY_FLY:
				foodhealthlayout.setVisibility(View.INVISIBLE);
				foodfoglay.setVisibility(View.INVISIBLE);
				break;
			case SmartSingleRenderer.FOOD_DISPLAY_HEALTH_INFO:
				int ax = (int) SmartNative.getCompositeResult()[5];
				int ay = GlobalParams.SCREEN_HEIGHT - (int) SmartNative.getCompositeResult()[4];
				int foodAttractiveScore = (int) (SmartNative.getFoodAttractiveness());
				foodhealthlayout.setVisibility(View.VISIBLE);
				foodfoglay.setVisibility(View.INVISIBLE);
				foodhealthlayout.setLeft(ax);
				foodhealthlayout.setTop(ay);
				textView1.setText(foodAttractiveScore + "%");// set textview
																// content
				ratingBar2.setRating(foodAttractiveScore / 20); // set ratingbar
				break;
			case SmartSingleRenderer.FOOD_MODLE_FLAG_VIDEO:
				foodfoglay.setVisibility(View.INVISIBLE);
				foodhealthlayout.setVisibility(View.INVISIBLE);
				break;
			default:
				break;
			}
		} else {
		}
	}
	/**
	 * show the food information
	 */
	private static void face_info_display() {
		if (CameraSurfaceView.faceFlag == true) {
			// display face score
			face.setVisibility(View.VISIBLE);
			int ax = CameraSurfaceView.faceLeftTag;
			int ay = CameraSurfaceView.faceTopTag;
			face.setLeft(ax);
			face.setTop(ay);
			age.setText(CameraSurfaceView.smilescore + "");
			faceValue.setText(CameraSurfaceView.facescore + "");
		} else {
			face.setVisibility(View.INVISIBLE);
		}
	}
	//handler to synchronize the food info display with the YUV callback data
	static Handler mHandler = new Handler() {  
        public void handleMessage(Message msg) {   
             switch (msg.what) {   
                  case MSG_UPDATE_UI: 
              		Log.i("SetSceneFlagTag", "Set Flag test 2, Scene: " + GlobalParams.sceneResultFlag
            				+ " , motion: " + GlobalParams.sceneMotionFlag);
                	 smartSceneInfoView.setSceneFlag(GlobalParams.sceneResultFlag, GlobalParams.sceneMotionFlag);
                	 smartSceneInfoView.invalidate();
          			if (CameraSurfaceView.sceneResult[0] != 8) {
        				SmartSingleRenderer.foodModelFlag = SmartSingleRenderer.FOOD_DISPLAY_HEALTH_INFO;
        				foodfoglay.setVisibility(View.INVISIBLE);
        				foodhealthlayout.setVisibility(View.INVISIBLE);
        				mylay.setVisibility(View.INVISIBLE);
        			}
        			food_health_display();
        			if (GlobalParams.OPEN_FACE) {
        				face_info_display();
        			} else {
        				face.setVisibility(View.INVISIBLE);
        			}
                  break;   
             }   
             super.handleMessage(msg);   
        }   
   };
   
	/**
	 * calling in CameraSurfaceView.java
	 */
	public static void updateUI()
	{
		mHandler.sendEmptyMessage(MSG_UPDATE_UI);
	}
	public void foodclick(View v) {
		int ax = (int) SmartNative.getCompositeResult()[5];
		int ay = GlobalParams.SCREEN_HEIGHT - (int) SmartNative.getCompositeResult()[4];
		int b = (int) (SmartNative.getDebugValue()[30]);
		if (mylay.getVisibility() == View.VISIBLE) {
			mylay.setVisibility(View.INVISIBLE);
		} else {
			mylay.setVisibility(View.VISIBLE);// set the framelayout into
												// visible
			textView1.setText(b + "%");// set textview content
			ratingBar2.setRating(b / 20); // set ratingbar
			foodhealthlayout.setLeft(ax);
			foodhealthlayout.setTop(ay);// set the position of button
		}
	}
	
	/**
	 * button to turn on/off ICG function
	 * @param v
	 */
	public void switchICG(View v) {
		if (GlobalParams.OPEN_ICG == false) {
			GlobalParams.OPEN_ICG = true;
			icgbutton.setText("ICG ON");
		} else {
			GlobalParams.OPEN_ICG = false;
			icgbutton.setText("ICG OFF");
		}
	}
	
	/**
	 * button to turn on/off tld function
	 * @param v
	 */
	public void resetTLD(View v) {
		if (GlobalParams.OPEN_TLD == false) {
			GlobalParams.OPEN_TLD = true;
		}

	}

	@Override
	public boolean onCreateOptionsMenu(Menu menu) {
		// TODO Auto-generated method stub
		MenuInflater inflater = getMenuInflater();
		inflater.inflate(R.menu.main, menu);
		return true;
	}

	@Override
	public boolean onOptionsItemSelected(MenuItem item) {
		// TODO Auto-generated method stub
		int item_id = item.getItemId();

		switch (item_id) {
		case R.id.action_stereo:
			Intent intent = new Intent(SmartSingleActivity.this, SmartStereoActivity.class);
			startActivity(intent);
			finish();
			break;
		case R.id.action_single:
			break;
		case R.id.action_open_face:
			GlobalParams.OPEN_FACE = true;
			if (!GlobalParams.isLoadSo) {
				System.loadLibrary("opencv_java");
				System.loadLibrary("CRBCamera");
				GlobalParams.isLoadSo = true;
			}
			break;
		case R.id.action_close_face:
			GlobalParams.OPEN_FACE = false;
			break;
		default:
			return false;
		}
		return true;
	}


	// this function below implements the response of click the dot
	public void faceclick(View v) {
		if (face_layout.getVisibility() == View.VISIBLE) {
			face_layout.setVisibility(View.INVISIBLE);
			GlobalParams.faceGLassFlag = true;
			// textView1.setText(b+"");
		} else {
			GlobalParams.faceGLassFlag = false;
			int ax = CameraSurfaceView.faceLeftTag;
			int ay = CameraSurfaceView.faceTopTag;
			face.setLeft(ax);
			face.setTop(ay);
			face_layout.setVisibility(View.VISIBLE);
			age.setText(CameraSurfaceView.scores[2] + "");
			faceValue.setText(CameraSurfaceView.scores[0] + "");
		}
	}

	@Override
	public void onBackPressed() {
		if (firstTime + 2000 > System.currentTimeMillis()) {
			super.onBackPressed();
			Intent startMain = new Intent(Intent.ACTION_MAIN);
			startMain.addCategory(Intent.CATEGORY_HOME);
			startMain.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
			startActivity(startMain);
			System.exit(0);
		} else {
			Toast.makeText(this, "再按一次退出程序", Toast.LENGTH_SHORT).show();
		}
		firstTime = System.currentTimeMillis();
	}
	

	public void saveData(String path, byte[] data) {
		try {
			File file = new File(path);
			FileOutputStream out = new FileOutputStream(file);
			out.write(data);
			out.flush();
			out.close();
		} catch (Exception e) {
			e.printStackTrace();
		}
	}

	public void saveData(String path, String log) {
		try {
			File file = new File(path);
			FileOutputStream out = new FileOutputStream(file, true);
			out.write(log.getBytes());
			out.flush();
			out.close();
		} catch (Exception e) {
			e.printStackTrace();
		}
	}

	public byte[] readData(String path) {
		byte[] data = null;
		try {
			File file = new File(path);
			FileInputStream in = new FileInputStream(file);
			data = new byte[in.available()];
			in.read(data);
			in.close();
		} catch (Exception e) {
			e.printStackTrace();
		}
		return data;
	}
}
