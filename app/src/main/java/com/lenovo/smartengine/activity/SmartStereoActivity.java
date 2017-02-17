package com.lenovo.smartengine.activity;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;

import android.annotation.TargetApi;
import android.content.Context;
import android.content.Intent;
import android.content.pm.ActivityInfo;
import android.content.res.Configuration;
import android.graphics.SurfaceTexture;
import android.hardware.Sensor;
import android.hardware.SensorEvent;
import android.hardware.SensorEventListener;
import android.hardware.SensorManager;
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
import android.widget.FrameLayout;
import android.widget.ImageButton;
import android.widget.LinearLayout;
import android.widget.RatingBar;
import android.widget.TextView;
import android.widget.Toast;

import com.example.arframetest.R;
import com.google.vrtoolkit.cardboard.CardboardActivity;
import com.google.vrtoolkit.cardboard.CardboardView;
import com.lenovo.smartengine.jni.SmartNative;
import com.lenovo.smartengine.util.GlobalParams;
import com.lenovo.smartengine.view.CameraSurfaceView;
import com.lenovo.smartengine.view.renderer.SmartSingleRenderer;
import com.lenovo.smartengine.view.renderer.SmartStereoRenderer;

public class SmartStereoActivity extends CardboardActivity implements
		SurfaceTexture.OnFrameAvailableListener, OnGestureListener {

	private static final String TAG = "nftSimple";

	public static final int MSG_UPDATE_UI = 0x01;
	public static final int ORIENTATION_HYSTERESIS = 5;
	public static final int FOOD_DISPLAY_HEALTH_INFO = 0;

	public static final int FACE_DISPLAY_INFO = 0;
	public static final int FACE_DISPLAY_GLASSES = 1;

	public static int modelFlag = FOOD_DISPLAY_HEALTH_INFO;
	public static int faceFlag = 0;

	private static long firstTime;
	long index = 0;

	private SensorManager sensorMgr;
	private OrientationListener oriListener;

	private int orientationRaw = 0;

	static LinearLayout foodhealthlayout;
	static FrameLayout mylay;
	static TextView textView1;
	static RatingBar ratingBar2;
	ImageButton foodbutton;
	public static FrameLayout foodfoglay;
	// private GL2SurfaceView glView;
	private CardboardView glView;
	private CameraSurfaceView camSurface;

	// lyp
	private static LinearLayout foodhealthlayout_right;
	private static FrameLayout mylay_right;
	private static TextView textView1_right;
	private static RatingBar ratingBar2_right;
	private ImageButton foodbutton_right;
	public static FrameLayout foodfoglay_right;

	private FrameLayout mainLayout;
	GestureDetector detector;

	private static FrameLayout face_layout;
	private static LinearLayout face;
	private static TextView age;
	private static TextView faceValue;
	private ImageButton facebutton;

	private static FrameLayout face_layout_right;
	private static LinearLayout face_right;
	private static TextView age_right;
	private static TextView faceValue_right;
	private ImageButton facebutton_right;

	static {
		if (GlobalParams.OPEN_FACE) {
			System.loadLibrary("opencv_java");
			System.loadLibrary("CRBCamera");
			GlobalParams.isLoadSo = true;
		}
	}
	
	static Handler mHandler = new Handler() {  
        public void handleMessage(Message msg) {   
             switch (msg.what) {   
                  case MSG_UPDATE_UI:  
          			if (CameraSurfaceView.sceneResult[0] != 8) {
        				SmartStereoRenderer.modelFlag = SmartStereoRenderer.FOOD_DISPLAY_HEALTH_INFO;
        				foodfoglay.setVisibility(View.INVISIBLE);
        				foodhealthlayout.setVisibility(View.INVISIBLE);
        				mylay.setVisibility(View.INVISIBLE);

        				// lyp
        				foodfoglay_right.setVisibility(View.INVISIBLE);
        				foodhealthlayout_right.setVisibility(View.INVISIBLE);
        				mylay_right.setVisibility(View.INVISIBLE);
        			} else {
        				// lyp
        				foodhealthlayout.setVisibility(View.VISIBLE);
        				mylay.setVisibility(View.VISIBLE);
        				foodhealthlayout_right.setVisibility(View.VISIBLE);
        				mylay_right.setVisibility(View.VISIBLE);
        			}
        			food_health_display();
        			if (GlobalParams.OPEN_FACE) {
        				face_info_display();
        			} else {
        				face.setVisibility(View.INVISIBLE);
        				face_right.setVisibility(View.INVISIBLE);
        			}
                    break;      
             }   
             super.handleMessage(msg);   
        }   
   };

	/** Called when the activity is first created. */
	@TargetApi(Build.VERSION_CODES.HONEYCOMB)
	@Override
	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		GlobalParams.INIT_ACTIVITY = 1;
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

		setContentView(R.layout.main_stereo);

		initManager(this);
		mainLayout = (FrameLayout) this.findViewById(R.id.mainLayout);
		foodfoglay = (FrameLayout) findViewById(R.id.food_fog_lay);
		mylay = (FrameLayout) findViewById(R.id.frameLayout1);
		textView1 = (TextView) findViewById(R.id.textView1);
		ratingBar2 = (RatingBar) findViewById(R.id.ratingBar2);
		foodbutton = (ImageButton) findViewById(R.id.foodbutton);
		foodhealthlayout = (LinearLayout) findViewById(R.id.food);

		// lyp
		mylay_right = (FrameLayout) findViewById(R.id.fl_health_right);
		textView1_right = (TextView) findViewById(R.id.tv_health_right);
		ratingBar2_right = (RatingBar) findViewById(R.id.rb_health_right);
		foodbutton_right = (ImageButton) findViewById(R.id.foodbutton_right);
		foodhealthlayout_right = (LinearLayout) findViewById(R.id.food_right);
		foodfoglay_right = (FrameLayout) findViewById(R.id.food_fog_lay_right);

		// face
		face_layout = (FrameLayout) findViewById(R.id.face_frameLayout);
		face = (LinearLayout) findViewById(R.id.face);
		age = (TextView) findViewById(R.id.age);
		faceValue = (TextView) findViewById(R.id.faceValue);
		facebutton = (ImageButton) findViewById(R.id.facebutton);
		// ryp
		face_layout_right = (FrameLayout) findViewById(R.id.face_frameLayout_right);
		face_right = (LinearLayout) findViewById(R.id.face_right);
		age_right = (TextView) findViewById(R.id.age_right);
		faceValue_right = (TextView) findViewById(R.id.faceValue_right);
		facebutton_right = (ImageButton) findViewById(R.id.facebutton_right);
	}

	@Override
	public void onStart() {
		super.onStart();
		GlobalParams.INIT_ACTIVITY = 1;
	}

	@SuppressWarnings("deprecation")
	// FILL_PARENT still required for API level 7 (Android 2.1)
	@Override
	public void onResume() {
		super.onResume();
		GlobalParams.INIT_ACTIVITY = 1;
		// Create the camera view.
		camSurface = new CameraSurfaceView(this);

		// Create/recreate the GL view.
		// glView = new GL2SurfaceView(this, true, false, 16, 0);
		glView = new CardboardView(this);
		glView.setEGLContextClientVersion(2);// add by mikey
		glView.setRenderer(new SmartStereoRenderer(this));
		setCardboardView(glView);

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

	private void updateNativeDisplayParameters() {
		Display d = getWindowManager().getDefaultDisplay();
		int orientation = d.getRotation();
		DisplayMetrics dm = new DisplayMetrics();
		d.getMetrics(dm);
		int w = dm.widthPixels;
		int h = dm.heightPixels;
		int dpi = dm.densityDpi;
	}

	@Override
	public void onConfigurationChanged(Configuration newConfig) {
		super.onConfigurationChanged(newConfig);

		updateNativeDisplayParameters();
	}

	@Override
	public void onFrameAvailable(SurfaceTexture surfaceTexture) {
		glView.requestRender();
	}

	@Override
	public boolean onTouchEvent(MotionEvent event) {
		int modelFlag = SmartStereoRenderer.modelFlag;
		detector.onTouchEvent(event);

		switch (event.getAction()) {

		case MotionEvent.ACTION_DOWN:
			SmartStereoRenderer.modelFlag++;
			SmartStereoRenderer.modelFlag = SmartStereoRenderer.modelFlag % 3;
			Log.e(TAG, "onTouchEvent:" + SmartStereoRenderer.modelFlag);
			faceFlag++;
			faceFlag = faceFlag % 2;
			if (GlobalParams.OPEN_FACE && CameraSurfaceView.faceFlag == true) {
				Log.e(TAG, "faceFlag:" + faceFlag);
				faceclick();
			}
			break;
		case MotionEvent.ACTION_MOVE:

			break;
		default:
			break;
		}

		return true;
	}

	public void initManager(Context context) {
		sensorMgr = (SensorManager) context
				.getSystemService(Context.SENSOR_SERVICE);
		oriListener = new OrientationListener(context);
	}

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

	private void unregisterSensor() {
		sensorMgr.unregisterListener(sensorListener);
		oriListener.disable();
	}

	// private int previewFlag = 0;
	// private boolean healthFlag = false;
	private SensorEventListener sensorListener = new SensorEventListener() {
		float[] aSensor = new float[3];
		float[] oSensor = new float[3];
		float[] lSensor = new float[3];

		@Override
		public void onSensorChanged(SensorEvent e) {
//			if (CameraSurfaceView.sceneResult[0] != 8) {
//				SmartStereoRenderer.modelFlag = SmartStereoRenderer.FOOD_DISPLAY_HEALTH_INFO;
//				foodfoglay.setVisibility(View.INVISIBLE);
//				foodhealthlayout.setVisibility(View.INVISIBLE);
//				mylay.setVisibility(View.INVISIBLE);
//
//				// lyp
//				foodfoglay_right.setVisibility(View.INVISIBLE);
//				foodhealthlayout_right.setVisibility(View.INVISIBLE);
//				mylay_right.setVisibility(View.INVISIBLE);
//			} else {
//				// lyp
//				foodhealthlayout.setVisibility(View.VISIBLE);
//				mylay.setVisibility(View.VISIBLE);
//				foodhealthlayout_right.setVisibility(View.VISIBLE);
//				mylay_right.setVisibility(View.VISIBLE);
//			}
//			food_health_display();
//			if (GlobalParams.OPEN_FACE) {
//				face_info_display();
//			} else {
//				face.setVisibility(View.INVISIBLE);
//				face_right.setVisibility(View.INVISIBLE);
//			}
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
			if (GlobalParams.OPEN_FACE) {
				GlobalParams.orientation = roundOrientation(ori,
						GlobalParams.orientation);
			}
		}
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
	public boolean onFling(MotionEvent e1, MotionEvent e2, float velocityX,
			float velocityY) {
		// TODO Auto-generated method stub

		if (e1.getX() - e2.getX() < 50) {
			SmartStereoRenderer.modelFlag++;
			if (SmartStereoRenderer.modelFlag > 2)
				SmartStereoRenderer.modelFlag = 2;
			SmartStereoRenderer.modelFlag = SmartStereoRenderer.modelFlag % 3;
		} else if (e1.getX() - e2.getX() > 50) {
			SmartStereoRenderer.modelFlag--;
			if (SmartStereoRenderer.modelFlag < 0)
				SmartStereoRenderer.modelFlag = 0;
			SmartStereoRenderer.modelFlag = SmartStereoRenderer.modelFlag % 3;
		}

		return true;
	}

	private static void food_health_display() {
		Log.i("TouchLog", "Touch: ");
		if (CameraSurfaceView.sceneResult[0] == 8) {
			switch (SmartStereoRenderer.modelFlag) {
			case SmartStereoRenderer.FOOD_DISPLAY_FOG:
				// lyp
				Log.i("lyp", "FOOD_DISPLAY_FOG");
				int left_position = SmartNative.getCompositeResult()[5] - 600;
				int right_position = SmartNative.getCompositeResult()[5] + 350;
				int top_position = GlobalParams.SCREEN_HEIGHT - SmartNative.getCompositeResult()[4] - 300;
				Log.i("lyp", "坐标偏移:" + left_position);
				if (left_position < 580) {
					foodfoglay.setLeft(left_position);
					foodfoglay.setTop(top_position);
					foodfoglay.setVisibility(View.VISIBLE);
				} else {
					foodfoglay.setVisibility(View.INVISIBLE);
				}
				if (right_position > 850) {
					foodfoglay_right.setLeft(right_position);
					foodfoglay_right.setTop(top_position);
					foodfoglay_right.setVisibility(View.VISIBLE);
				} else {
					foodfoglay_right.setVisibility(View.INVISIBLE);
				}
				foodhealthlayout.setVisibility(View.INVISIBLE);
				foodhealthlayout_right.setVisibility(View.INVISIBLE);
				break;
			case SmartStereoRenderer.FOOD_DISPLAY_FLY:
				// lyp
				foodhealthlayout.setVisibility(View.INVISIBLE);
				foodhealthlayout_right.setVisibility(View.INVISIBLE);
				foodfoglay.setVisibility(View.INVISIBLE);
				foodfoglay_right.setVisibility(View.INVISIBLE);
				break;
			case SmartStereoRenderer.FOOD_DISPLAY_HEALTH_INFO:
				// lyp
				int ax = (int) SmartNative.getCompositeResult()[5];
				int ay = GlobalParams.SCREEN_HEIGHT - (int) SmartNative.getCompositeResult()[4];
				int foodAttractiveScore = (int) (SmartNative
						.getFoodAttractiveness());
				if (ax - 600 < 680) {
					foodhealthlayout.setVisibility(View.VISIBLE);
					foodfoglay.setVisibility(View.INVISIBLE);
					foodhealthlayout.setLeft(ax - 600);
					foodhealthlayout.setTop(ay);
					textView1.setText(foodAttractiveScore + "%");// set textview
					ratingBar2.setRating(foodAttractiveScore / 20); // set
				} else {
					foodhealthlayout.setVisibility(View.INVISIBLE);
				}
				if (ax + 400 > 850) {
					foodhealthlayout_right.setVisibility(View.VISIBLE);
					foodfoglay_right.setVisibility(View.INVISIBLE);
					foodhealthlayout_right.setLeft(ax + 350);
					foodhealthlayout_right.setTop(ay);
					textView1_right.setText(foodAttractiveScore + "%");// set
					ratingBar2_right.setRating(foodAttractiveScore / 20); // set
				} else {
					foodhealthlayout_right.setVisibility(View.INVISIBLE);
				}
				break;
			default:
				break;
			}
		} else {
		}
	}

	public void foodclick(View v) {
		int ax = (int) SmartNative.getCompositeResult()[5];
		int ay = GlobalParams.SCREEN_HEIGHT - (int) SmartNative.getCompositeResult()[4];
		int b = (int) (SmartNative.getDebugValue()[30]);
		if (mylay.getVisibility() == View.VISIBLE) {
			mylay.setVisibility(View.INVISIBLE);
		} else {
			mylay.setVisibility(View.VISIBLE);// set the framelayout into
			textView1.setText(b + "%");// set textview content
			ratingBar2.setRating(b / 20); // set ratingbar
			foodhealthlayout.setLeft(ax - 600);
			foodhealthlayout.setTop(ay);// set the position of button
		}// lyp
		if (mylay_right.getVisibility() == View.VISIBLE) {
			mylay_right.setVisibility(View.INVISIBLE);
		} else {
			mylay_right.setVisibility(View.VISIBLE);// set the framelayout into
			// visible
			textView1_right.setText(b + "%");// set textview content
			ratingBar2_right.setRating(b / 20); // set ratingbar
			foodhealthlayout_right.setLeft(ax + 350);
			foodhealthlayout_right.setTop(ay);// set the position of button
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
			break;
		case R.id.action_single:
			Intent intent = new Intent(SmartStereoActivity.this,
					SmartSingleActivity.class);
			startActivity(intent);
			finish();
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

	private static void face_info_display() {
		if (CameraSurfaceView.faceFlag == true && faceFlag == FACE_DISPLAY_INFO) {
			// display face score

			int ax = CameraSurfaceView.faceLeftTag + 200;
			int ay = CameraSurfaceView.faceTopTag;
			if (ax - 600 < 680) {
				face.setLeft(ax - 600);
				face.setTop(ay);
				face.setVisibility(View.VISIBLE);
				face_layout.setVisibility(View.VISIBLE);
				age.setText(CameraSurfaceView.smilescore + "");
				faceValue.setText(CameraSurfaceView.facescore + "");
			} else {
				face.setVisibility(View.INVISIBLE);
				face_layout.setVisibility(View.INVISIBLE);
			}
			if (ax + 400 > 850) {
				face_right.setLeft(ax + 350);
				face_right.setTop(ay);
				face_right.setVisibility(View.VISIBLE);
				face_layout_right.setVisibility(View.VISIBLE);
				age_right.setText(CameraSurfaceView.smilescore + "");
				faceValue_right.setText(CameraSurfaceView.facescore + "");
			} else {
				face_right.setVisibility(View.INVISIBLE);
				face_layout_right.setVisibility(View.INVISIBLE);
			}
		} else {
			face.setVisibility(View.INVISIBLE);
			face_layout.setVisibility(View.INVISIBLE);
			face_right.setVisibility(View.INVISIBLE);
			face_layout_right.setVisibility(View.INVISIBLE);
		}
	}

	// this function below implements the response of click the dot
	public void faceclick() {
		if (faceFlag == FACE_DISPLAY_INFO) {
			Log.e(TAG, "lyp:FACE_DISPLAY_INFO,flag:false,faceflag:" + faceFlag);
			GlobalParams.faceGLassFlag = false;
			int ax = CameraSurfaceView.faceLeftTag + 200;
			int ay = CameraSurfaceView.faceTopTag;

			if (ax - 600 < 680) {
				face.setLeft(ax - 600);
				face.setTop(ay);
				face.setVisibility(View.VISIBLE);
				face_layout.setVisibility(View.VISIBLE);
				age.setText(CameraSurfaceView.scores[2] + "");
				faceValue.setText(CameraSurfaceView.scores[0] + "");
			} else {
				face.setVisibility(View.INVISIBLE);
				face_layout.setVisibility(View.INVISIBLE);
			}
			if (ax + 400 > 850) {
				face_right.setLeft(ax + 350);
				face_right.setTop(ay);
				face_right.setVisibility(View.VISIBLE);
				face_layout_right.setVisibility(View.VISIBLE);
				age_right.setText(CameraSurfaceView.scores[2] + "");
				faceValue_right.setText(CameraSurfaceView.scores[0] + "");
			} else {
				face_right.setVisibility(View.VISIBLE);
				face_layout_right.setVisibility(View.VISIBLE);
			}

		} else if (faceFlag == FACE_DISPLAY_GLASSES) {
			Log.e(TAG, "FACE_DISPLAY_GLASSES,flag:true,faceflag:" + faceFlag);
			face.setVisibility(View.INVISIBLE);
			face_layout.setVisibility(View.INVISIBLE);
			face_right.setVisibility(View.INVISIBLE);
			face_layout_right.setVisibility(View.INVISIBLE);
			GlobalParams.faceGLassFlag = true;
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
	
	public static void updateUI()
	{
		mHandler.sendEmptyMessage(MSG_UPDATE_UI);
	}
}
