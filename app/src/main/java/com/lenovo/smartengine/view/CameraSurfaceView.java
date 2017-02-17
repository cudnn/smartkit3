/*
 *  CameraSurface.java
 *  ARToolKit5
 *
 *  Disclaimer: IMPORTANT:  This Daqri software is supplied to you by Daqri
 *  LLC ("Daqri") in consideration of your agreement to the following
 *  terms, and your use, installation, modification or redistribution of
 *  this Daqri software constitutes acceptance of these terms.  If you do
 *  not agree with these terms, please do not use, install, modify or
 *  redistribute this Daqri software.
 *
 *  In consideration of your agreement to abide by the following terms, and
 *  subject to these terms, Daqri grants you a personal, non-exclusive
 *  license, under Daqri's copyrights in this original Daqri software (the
 *  "Daqri Software"), to use, reproduce, modify and redistribute the Daqri
 *  Software, with or without modifications, in source and/or binary forms;
 *  provided that if you redistribute the Daqri Software in its entirety and
 *  without modifications, you must retain this notice and the following
 *  text and disclaimers in all such redistributions of the Daqri Software.
 *  Neither the name, trademarks, service marks or logos of Daqri LLC may
 *  be used to endorse or promote products derived from the Daqri Software
 *  without specific prior written permission from Daqri.  Except as
 *  expressly stated in this notice, no other rights or licenses, express or
 *  implied, are granted by Daqri herein, including but not limited to any
 *  patent rights that may be infringed by your derivative works or by other
 *  works in which the Daqri Software may be incorporated.
 *
 *  The Daqri Software is provided by Daqri on an "AS IS" basis.  DAQRI
 *  MAKES NO WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION
 *  THE IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS
 *  FOR A PARTICULAR PURPOSE, REGARDING THE DAQRI SOFTWARE OR ITS USE AND
 *  OPERATION ALONE OR IN COMBINATION WITH YOUR PRODUCTS.
 *
 *  IN NO EVENT SHALL DAQRI BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL
 *  OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 *  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 *  INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION,
 *  MODIFICATION AND/OR DISTRIBUTION OF THE DAQRI SOFTWARE, HOWEVER CAUSED
 *  AND WHETHER UNDER THEORY OF CONTRACT, TORT (INCLUDING NEGLIGENCE),
 *  STRICT LIABILITY OR OTHERWISE, EVEN IF DAQRI HAS BEEN ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.
 *
 *  Copyright 2015 Daqri, LLC.
 *  Copyright 2011-2015 ARToolworks, Inc.
 *
 *  Author(s): Julian Looser, Philip Lamb
 *
 */

package com.lenovo.smartengine.view;

import java.io.ByteArrayOutputStream;
import java.io.Closeable;
import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.util.List;

import org.opencv.android.Utils;
import org.opencv.core.CvType;
import org.opencv.core.Mat;
import org.opencv.core.Size;
import org.opencv.imgproc.Imgproc;

import android.annotation.SuppressLint;
import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.ImageFormat;
import android.graphics.PixelFormat;
import android.graphics.Rect;
import android.graphics.YuvImage;
import android.hardware.Camera;
import android.hardware.Camera.AutoFocusCallback;
import android.hardware.Camera.Face;
import android.hardware.Camera.FaceDetectionListener;
import android.hardware.Camera.Parameters;
//import android.hardware.Camera.Q3aDataCallback;
import android.os.Build;
import android.preference.PreferenceManager;
import android.util.Log;
import android.view.SurfaceHolder;
import android.view.SurfaceView;

import com.lenovo.smartengine.face;
import com.lenovo.smartengine.activity.SmartSingleActivity;
//import com.lenovo.smartengine.activity.SmartStereoActivity;
import com.lenovo.smartengine.gl.model.GLSmartYUV2D;
import com.lenovo.smartengine.jni.SmartNative;
import com.lenovo.smartengine.util.Debug;
import com.lenovo.smartengine.util.GlobalParams;
import com.lenovo.smartengine.view.renderer.SmartSingleRenderer;
import com.lenovo.smartengine.view.renderer.SmartStereoRenderer;
import com.onesun.FaceAttractiveness;
import com.onesun.MainActivity;

/***
 * factt
 * 
 * @author Leo Tyndale
 * 
 */
public class CameraSurfaceView extends SurfaceView implements SurfaceHolder.Callback, FaceDetectionListener {

	private static final String TAG = "CameraSurface";
	public static Camera camera;
	private MainActivity mActivity;
	
	public static int preview_height = 720;//960;//
	public static int preview_width = 1280;
	private static final float size1_ThresLow = 1.72f,     //16:9
							   size1_ThresHigh = 1.82f,
							   size2_ThresLow = 1.28f,     //4:3
							   size2_ThresHigh = 1.38f;
	
	private int h = 202;
	private int w = 360;
	
	public static int faceTopTag;
	public static int faceLeftTag;
	private Face[] facedata;

	private int[] param1 = new int[] { 100, 120, 95, 105, 6, 3 , 394 ,1, 500};
	private float[] param2 = new float[] { 0.1f, 2 / 3f, 0.22f, 0.06f, 0.02f, 0.08f, 0.04f, 0.05f , 0.9f};
//	private int[] param1 = new int[] { 100, 120, 105, 90, 6, 3, 395, 1 };
//	private float[] param2 = new float[] { 0.1f, 2 / 3f, 0.22f, 0.06f, 0.02f, 0.08f, 0.04f, 0.05f, 0.9f };
	private boolean isSmartEnable = true;

	public static int[] track_roi = new int[]{ 0,0,0,0,0 };
	
	@SuppressWarnings("deprecation")
	public CameraSurfaceView(Context context) {

		super(context);

		SurfaceHolder holder = getHolder();
		holder.addCallback(this);
		holder.setType(SurfaceHolder.SURFACE_TYPE_PUSH_BUFFERS);

	}

	// SurfaceHolder.Callback methods

	@SuppressLint("NewApi")
	@Override
	public void surfaceCreated(SurfaceHolder holder) {

		Log.i(TAG, "Opening camera.");
		try {
			if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.GINGERBREAD) {
				int cameraIndex = Integer.parseInt(PreferenceManager
						.getDefaultSharedPreferences(getContext()).getString(
								"pref_cameraIndex", "0"));
				camera = Camera.open(cameraIndex);
			} else {
				camera = Camera.open();
			}
		} catch (RuntimeException exception) {
			Log.e(TAG,
					"Cannot open camera. It may be in use by another process.");
		}
		
		if (camera != null) {
			float preivewRatio = (float)preview_width/preview_height;
			if(preivewRatio > size1_ThresLow && preivewRatio < size1_ThresHigh){
				GlobalParams.preivew_16_9_flag = true;
			}else if(preivewRatio > size2_ThresLow && preivewRatio < size2_ThresHigh){
				GlobalParams.preivew_16_9_flag = false;
				GlobalParams.SCREEN_WIDTH = GlobalParams.SCREEN_HEIGHT * 4 / 3;
			}
			Camera.Parameters parameters = camera.getParameters();
			List<Camera.Size> previewsize = parameters.getSupportedPreviewSizes();
			parameters.setPreviewSize(preview_width, preview_height);
			// parameters.setPreviewSize(1920, 1080);
			// parameters.setPreviewSize(Integer.parseInt(dims[0]),
			// Integer.parseInt(dims[1]));
			parameters.setPreviewFrameRate(30);
			List<Integer> list_preview = parameters.getSupportedPreviewFormats();
			camera.setParameters(parameters);

			parameters = camera.getParameters();
			int capWidth = parameters.getPreviewSize().width;
			int capHeight = parameters.getPreviewSize().height;
			int pixelformat = parameters.getPreviewFormat(); // android.graphics.imageformat
			PixelFormat pixelinfo = new PixelFormat();
			PixelFormat.getPixelFormatInfo(pixelformat, pixelinfo);
			int cameraIndex = 0;
			boolean frontFacing = false;
			if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.GINGERBREAD) {
				Camera.CameraInfo cameraInfo = new Camera.CameraInfo();
				cameraIndex = Integer.parseInt(PreferenceManager.getDefaultSharedPreferences(getContext()).getString("pref_cameraIndex", "0"));
				Camera.getCameraInfo(cameraIndex, cameraInfo);
				if (cameraInfo.facing == Camera.CameraInfo.CAMERA_FACING_FRONT)
					frontFacing = true;
			}

			int bufSize = capWidth * capHeight * pixelinfo.bitsPerPixel / 8; // For

			for (int i = 0; i < 5; i++)
				camera.addCallbackBuffer(new byte[bufSize]);
			
			camera.startPreview();
//			camera.autoFocus(new AutoFocusCallback()
//			{
//
//				@Override
//				public void onAutoFocus(boolean success, Camera camera) {
//					// TODO Auto-generated method stub
//					
//				}
//				
//			});
//				
//			camera.setFaceDetectionListener(this);
//			camera.startFaceDetection();
		}
		
		if (camera != null) {
			try {

				camera.setPreviewDisplay(holder);
				// camera.setPreviewCallback(this);//start the onPreviewFrame function
				camera.setPreviewCallbackWithBuffer(new YUVPreviewCallback(mActivity, camera)); // API level 8 (Android 2.2)
				// camera.setPreviewCallbackWithBuffer(this); // API level 8
				// (Android 2.2)
				//camera.setQ3aDataCallback(new ISPDataCallback()); //set isp data 160314

			} catch (IOException exception) {
				Log.e(TAG, "Cannot set camera preview display.");
				camera.release();
				camera = null;
			}
		}

		Log.v("SMART_PLANET","1-["+GlobalParams.SCREEN_HEIGHT+" , "+GlobalParams.SCREEN_WIDTH+"],["+preview_height+" , "+preview_width+"]");
//		SmartNative.setScreenSize( GlobalParams.SCREEN_WIDTH , GlobalParams.SCREEN_HEIGHT);  // CHANGE THE ORDER FROM H-W TO W-H
		SmartNative.setScreenSize(GlobalParams.SCREEN_HEIGHT, GlobalParams.SCREEN_WIDTH);
		Log.v("SMART_PLANET","2-["+GlobalParams.SCREEN_HEIGHT+" , "+GlobalParams.SCREEN_WIDTH+"],["+preview_height+" , "+preview_width+"]");
		SmartNative.setPreviewSize(preview_width, preview_height);
		Log.v("SMART_PLANET","3-["+GlobalParams.SCREEN_HEIGHT+" , "+GlobalParams.SCREEN_WIDTH+"],["+preview_height+" , "+preview_width+"]");
		SmartNative.setSmartParameter(param1, param1.length, param2, param2.length);
		if (!Debug.ENABLE) {
			bmpPreview = Bitmap.createBitmap(preview_width, preview_height,
					Bitmap.Config.ARGB_8888);
		}
	}

	@Override
	public void surfaceDestroyed(SurfaceHolder holder) {

		if (camera != null) {
			Log.i(TAG, "Closing camera.");
			camera.stopPreview();
			camera.setPreviewCallback(null);
			camera.setPreviewCallbackWithBuffer(null);
			camera.release();
			camera = null;
		}
	}

	@SuppressLint("NewApi")
	// CameraInfo
	@SuppressWarnings("deprecation")
	// setPreviewFrameRate
	@Override
	public void surfaceChanged(SurfaceHolder holder, int format, int w, int h) {

//		if (camera != null) {
//			float preivewRatio = (float)preview_width/preview_height;
//			if(preivewRatio > size1_ThresLow && preivewRatio < size1_ThresHigh){
//				GlobalParams.preivew_16_9_flag = true;
//			}else if(preivewRatio > size2_ThresLow && preivewRatio < size2_ThresHigh){
//				GlobalParams.preivew_16_9_flag = false;
//			}
//			Camera.Parameters parameters = camera.getParameters();
//			List<Camera.Size> previewsize = parameters.getSupportedPreviewSizes();
//			parameters.setPreviewSize(preview_width, preview_height);
//			// parameters.setPreviewSize(1920, 1080);
//			// parameters.setPreviewSize(Integer.parseInt(dims[0]),
//			// Integer.parseInt(dims[1]));
//			parameters.setPreviewFrameRate(30);
//			camera.setParameters(parameters);
//
//			parameters = camera.getParameters();
//			int capWidth = parameters.getPreviewSize().width;
//			int capHeight = parameters.getPreviewSize().height;
//			int pixelformat = parameters.getPreviewFormat(); // android.graphics.imageformat
//			PixelFormat pixelinfo = new PixelFormat();
//			PixelFormat.getPixelFormatInfo(pixelformat, pixelinfo);
//			int cameraIndex = 0;
//			boolean frontFacing = false;
//			if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.GINGERBREAD) {
//				Camera.CameraInfo cameraInfo = new Camera.CameraInfo();
//				cameraIndex = Integer.parseInt(PreferenceManager
//						.getDefaultSharedPreferences(getContext()).getString(
//								"pref_cameraIndex", "0"));
//				Camera.getCameraInfo(cameraIndex, cameraInfo);
//				if (cameraInfo.facing == Camera.CameraInfo.CAMERA_FACING_FRONT)
//					frontFacing = true;
//			}
//
//			int bufSize = capWidth * capHeight * pixelinfo.bitsPerPixel / 8; // For
//
//			for (int i = 0; i < 5; i++)
//				camera.addCallbackBuffer(new byte[bufSize]);
//			
//			camera.startPreview();
//			camera.autoFocus(new AutoFocusCallback()
//			{
//
//				@Override
//				public void onAutoFocus(boolean success, Camera camera) {
//					// TODO Auto-generated method stub
//					
//				}
//				
//			});
//				
//			camera.setFaceDetectionListener(this);
//			camera.startFaceDetection();
//		}
	}

	public static void closeQuietly(Closeable stream) {
		if (stream != null) {
			try {
				stream.close();
			} catch (IOException e) {
				// ignore
			}
		}
	}

	private boolean threadFlag = false;
	private Bitmap bmpPreview;
	public static int previewWidth;
	public static int previewHeight;
	// public static Bitmap bmp1;
	// public static IntBuffer bmpTempBuffer;
	private FaceAttractiveness cFaceAttractiveness = new FaceAttractiveness();

	// Callback Data
	class YUVPreviewCallback implements Camera.PreviewCallback {
		private Context context;
		private Camera cameraProxy;
		private int width;
		private int height;
		private byte[] callbackBuffer;
		private byte[] processBuffer;
		private Object object = new Object();
		private boolean isRunning;
//		private int miPluginId = 10;
//		private int[] SmartParam1 = new int[] { 100, 120, 90, 105, 6, 3, 394,1, 450 };
//		private float[] SmartParam2 = new float[] { 0.1f, 2 / 3f, 0.22f, 0.06f,0.02f, 0.08f, 0.04f, 0.05f, 0.9f };

		public YUVPreviewCallback(Context context, Camera cameraDevice) {
			this.context = context;
			this.cameraProxy = cameraDevice;
			if (camera != null) {
				android.hardware.Camera camera1 = camera;
				Parameters parameter = camera1.getParameters();
				android.hardware.Camera.Size size = parameter.getPreviewSize();
				this.width = size.width;
				this.height = size.height;
				previewWidth = size.width;
				previewHeight = size.height;
				this.callbackBuffer = new byte[width * height * 3 / 2];
				this.processBuffer = new byte[width * height * 3 / 2];
				isRunning = true;
				if (GlobalParams.OPEN_FACE) {
					startRecognitionThread();
				}
			}
		}

		/* (non-Javadoc)
		 * get YUV preview callback data and do the scene detection and composition guide
		 * @see android.hardware.Camera.PreviewCallback#onPreviewFrame(byte[], android.hardware.Camera)
		 */
		@Override
		public void onPreviewFrame(byte[] data, Camera camera) {
			callbackBuffer = data;
			Log.i("FaceGlassLog", "Pre time2: " + GlobalParams.orientation + " scene: " + CameraSurfaceView.sceneResult[0]);
			getRectForAlwaysInFocus();
			if (faceFlag && GlobalParams.orientation == 270
/*					&& CameraSurfaceView.sceneResult[0] != 8
					&& CameraSurfaceView.sceneResult[0] != 9*/) {
				// ---faceFlag glass-------
				// byte[] dataYUV = readData("/sdcard/yuv/srcimg.yuv");
				// YuvImage yuv = new YuvImage(dataYUV, ImageFormat.NV21, w, h,
				// null);
				// ////////////////////////////////////////////////////////////////yub2bmp
				Log.i("FaceGlassLog", "Pre time1: ");
				if (GlobalParams.faceGLassFlag) {

					long start = System.currentTimeMillis();

					YuvImage yuv;
					ByteArrayOutputStream stream;
					Mat SRCMat;

					if (Debug.ENABLE) {
						Log.i("FaceGlassLog", "Pre time3: ");
						yuv = new YuvImage(data, ImageFormat.NV21,preview_width, preview_height, null);
						stream = new ByteArrayOutputStream();
						yuv.compressToJpeg(new Rect(0, 0, preview_width,preview_height), 100, stream);
						bmpPreview = BitmapFactory.decodeByteArray(stream.toByteArray(), 0, stream.size());
						// YUV data downsize
						SRCMat = new Mat();
						Utils.bitmapToMat(bmpPreview, SRCMat);
						Imgproc.resize(SRCMat, SRCMat, new Size(w, h));
						Imgproc.cvtColor(SRCMat, SRCMat, Imgproc.COLOR_BGR2RGB);
					} else {
						// ------ data to mat directly----------
						bmpPreview = Bitmap.createBitmap(preview_width,
								preview_height, Bitmap.Config.ARGB_8888);
						sRCMatcopyMat = new Mat(preview_height * 3 / 2,
								preview_width, CvType.CV_8UC1);
						sRCMatcopyMat.put(0, 0, data);
						SRCMat = new Mat(preview_height, preview_width,
								CvType.CV_8UC4);
						Imgproc.cvtColor(sRCMatcopyMat, SRCMat,
								Imgproc.COLOR_YUV2RGB_NV12);// callback data
															// NV12
						Imgproc.resize(SRCMat, SRCMat, new Size(w, h));
					}

					long dataPreproc = System.currentTimeMillis() - start;

					start = System.currentTimeMillis();

					// Mat WarpMat = new Mat();
					// landmark = face.FacialAttractiveness(
					// SRCMat.getNativeObjAddr(), faceleft, faceright,
					// facetop, facebottom);
					// face.ARGlass(SRCMat.getNativeObjAddr(), faceleft,
					// faceright, facetop, facebottom,
					// WarpMat.getNativeObjAddr(), landmark);
					Mat WarpMat = new Mat(h, w, CvType.CV_8UC3);
					face.ARGlass(SRCMat.getNativeObjAddr(), faceleft,
							faceright, facetop, facebottom,
							WarpMat.getNativeObjAddr());
					long dataProc = System.currentTimeMillis() - start;
					start = System.currentTimeMillis();

					Imgproc.cvtColor(SRCMat, SRCMat, Imgproc.COLOR_RGB2BGR);
					Imgproc.resize(SRCMat, SRCMat, new Size(preview_width,
							preview_height));
					Utils.matToBitmap(SRCMat, bmpPreview);

					// the wrong way of setting
					if (GlobalParams.INIT_ACTIVITY == 0) {
						SmartSingleRenderer.mRGBRenderer.setBitmap(bmpPreview,
								bmpPreview.getHeight(), bmpPreview.getWidth());
					} else {
						SmartStereoRenderer.mRGBRenderer.setBitmap(bmpPreview,
								bmpPreview.getHeight(), bmpPreview.getWidth());
					}

					long dataPostProc = System.currentTimeMillis() - start;
					start = System.currentTimeMillis();

					// MainActivity.nativeVideoFrame(yuvtest, preview_width,
					// preview_height);
					long dataToTex = System.currentTimeMillis() - start;
					Log.i("FaceGlassLog", "Pre time: " + dataPreproc
							+ " Proc time: " + dataProc + " Post time: "
							+ dataPostProc + " Trans time: " + dataToTex);
					// MainActivity.nativeVideoFrame(dataYUV, preview_width,
					// preview_height);
				} else {					
					if (isSmartEnable) {
						long start = System.currentTimeMillis();
//						int result = SmartNative.sceneDetector(data);
						// int[] debugData = IVCSceneNative.getDebugData();
//						sceneResult = SmartNative.getSceneResult();
						Log.i("ScenedebugLog", "Scene: " + sceneResult[0]	+ ", Motion: " + sceneResult[1]);
						GlobalParams.sceneResultFlag = sceneResult[0];
						GlobalParams.sceneMotionFlag = sceneResult[1];
	              		Log.i("SetSceneFlagTag", "Set Flag test 3, Scene: " + GlobalParams.sceneResultFlag	+ " , motion: " + GlobalParams.sceneMotionFlag);
						// int[] qrPosition = IVCSceneNative.getQRCodePosition();
						// float[] debugValue = IVCSceneNative.getDebugValue();
					}
					GLSmartYUV2D.setFrameData(data, preview_width,preview_height);
				}
			} else {
				// Scene recognition do here 170215
				if (isSmartEnable) {
					
					if(GlobalParams.OPEN_TLD == true){
						SmartNative.setTrigger(true);
						GlobalParams.OPEN_TLD = false;
					}
					
					long start = System.currentTimeMillis();
					int result = SmartNative.sceneDetector(data);
					// int[] debugData = IVCSceneNative.getDebugData();
					sceneResult = SmartNative.getSceneResult();
					Log.i("ScenedebugLog", "Scene: " + sceneResult[0] + ", Motion: " + sceneResult[1]);
					GlobalParams.sceneResultFlag = sceneResult[0];
					GlobalParams.sceneMotionFlag = sceneResult[1];
              		Log.i("SetSceneFlagTag", "Set Flag test 3, Scene: " + GlobalParams.sceneResultFlag	+ " , motion: " + GlobalParams.sceneMotionFlag);
					// int[] qrPosition = IVCSceneNative.getQRCodePosition();
					// float[] debugValue = IVCSceneNative.getDebugValue();
				}
				GLSmartYUV2D.setFrameData(data, preview_width, preview_height);
				// MainActivity.nativeVideoFrame(data, preview_width, preview_height);
			}

			// synchronized (object) {
			// object.notify();
			// }
			synchronized (object) {
				if (threadFlag) {
					System.arraycopy(data, 0, processBuffer, 0, data.length);
				}
				object.notify();
			}

			// send message to main thread to update ui
			if (GlobalParams.INIT_ACTIVITY == 0) {
				SmartSingleActivity.updateUI();
			} else {
//				SmartStereoActivity.updateUI();
			}
			camera.addCallbackBuffer(callbackBuffer);
		}

		byte[] test;

		private void startRecognitionThread() {
			new Thread(new Runnable() {
				@Override
				public void run() {
					while (isRunning) {
						try {
							synchronized (object) {
								object.wait();
							}
						} catch (InterruptedException e) {
							e.printStackTrace();
						}
						threadFlag = false;
						YuvImage yuv = new YuvImage(processBuffer,
								ImageFormat.NV21, preview_width,
								preview_height, null);
						// YuvImage yuv = new YuvImage(callbackBuffer,
						// ImageFormat.NV21, preview_width, preview_height,
						// null);
						ByteArrayOutputStream stream = new ByteArrayOutputStream();
						yuv.compressToJpeg(new Rect(0, 0, preview_width,
								preview_height), 100, stream);
						Bitmap bmp1 = BitmapFactory.decodeByteArray(
								stream.toByteArray(), 0, stream.size());

						// YUV data downsize
						Mat SRCMatThread = new Mat();
						Utils.bitmapToMat(bmp1, SRCMatThread);
						Imgproc.resize(SRCMatThread, SRCMatThread, new Size(w,
								h));

						Imgproc.cvtColor(SRCMatThread, SRCMatThread,
								Imgproc.COLOR_BGR2RGB);
						// faceFlag.ARGlass(SRCMat.getNativeObjAddr(),faceleft,faceright,facetop,facebottom);
						int[] landmark = new int[136];
						landmark = face.FacialAttractiveness(
								SRCMatThread.getNativeObjAddr(), faceleft,
								faceright, facetop, facebottom);
						facescore = cFaceAttractiveness
								.FaceAttractivenessScores(landmark);
						smilescore = face.SmileEvaluation(
								SRCMatThread.getNativeObjAddr(), faceleft,
								faceright, facetop, facebottom);

						blinkflag = face.BlinkDetect(
								SRCMatThread.getNativeObjAddr(), faceleft,
								faceright, facetop, facebottom);
						Log.d("Cameratest", "BlinkDetect flag is :" + blinkflag);
						threadFlag = true;
					}
				}
			}).start();
		}
	}

	public Mat sRCMatcopyMat;
	public static int[] scores = new int[5];
	public static int[] landmark = new int[136];
	public static float facescore;
	public static int smilescore;
	public static boolean blinkflag;
	// Camera.PreviewCallback methods.
	public static int[] sceneResult = new int[2];

//	@Override
//	public void onPreviewFrame(byte[] data, Camera cam) {
//	}

	int faceleftdummy = -1000;
	int facerightdummy = -1000;
	int facetopdummy = -1000;
	int facebottomdummy = -1000;

	int faceleft = 0;
	int faceright = 0;
	int facetop = 0;
	int facebottom = 0;
	
	public static int faceLength = 0;
	public static int[] faceData = new int[21];
	public static int[] faceDrawData = new int[21];
	public static boolean faceFlag = false;

	@Override
	public void onFaceDetection(Face[] faces, Camera arg1) {
		// TODO Auto-generated method stub

		Log.i("FaceDataLog", "-------> ");
		if (faces == null) {
			faceFlag = false;
			GlobalParams.faceGLassFlag = false;
			return;
		} else if (faces.length == 1) {
			faceFlag = true;
			faceleftdummy = faces[0].rect.left;
			facerightdummy = faces[0].rect.right;
			facetopdummy = faces[0].rect.top;
			facebottomdummy = faces[0].rect.bottom;

			faceTopTag = (facetopdummy * GlobalParams.SCREEN_HEIGHT / 2000) + GlobalParams.SCREEN_HEIGHT
					/ 2;
			faceLeftTag = (faceleftdummy * GlobalParams.SCREEN_WIDTH / 2000) + GlobalParams.SCREEN_WIDTH
					/ 2;
			faceleft = (faceleftdummy * w / 2000) + w / 2;
			faceright = (facerightdummy * w / 2000) + w / 2;
			facetop = (facetopdummy * h / 2000) + h / 2;
			facebottom = (facebottomdummy * h / 2000) + h / 2;
			Log.i("FaceDataLog", "Left: " + faceleft + ", Right: " + faceright
					+ ", Top: " + facetop + ", Bottom: " + facebottom);
		} else {
			faceFlag = false;
		}
		
		if (faces != null && faces.length > 0) {
			faceLength = faces.length;
			for (int i = 0; i < faceLength; i++) {
				faceData[4*i + 0] = faces[i].rect.left;
				faceData[4*i + 1] = faces[i].rect.top;
				faceData[4*i + 2] = faces[i].rect.right;
				faceData[4*i + 3] = faces[i].rect.bottom;
				faceDrawData[4*i + 0] = (faces[i].rect.left + 1000)*GlobalParams.SCREEN_WIDTH / 2000;
				faceDrawData[4*i + 1] = (faces[i].rect.top + 1000)*GlobalParams.SCREEN_HEIGHT / 2000;
				faceDrawData[4*i + 2] = (faces[i].rect.right + 1000)*GlobalParams.SCREEN_WIDTH / 2000;
				faceDrawData[4*i + 3] = (faces[i].rect.bottom + 1000)*GlobalParams.SCREEN_HEIGHT / 2000;
			}
		}else{
			faceLength = 0;
		}
		
		Log.i("FaceDataTest", "Face num: " + faceLength);
		SmartNative.setFaceData(faceData, faces.length);
		
	}

	public void getRectForAlwaysInFocus(){
		
		Log.i("tracking", "1:"+ track_roi[0] + " , "+track_roi[1] + " , "+track_roi[2]+ " , "+track_roi[3]+ " , "+track_roi[4]);
		
		if(track_roi[0] == GlobalParams.TRACKING_STATUS_INIT)
		{
			Log.i("tracking", "2:"+ track_roi[0] + " , "+track_roi[1] + " , "+track_roi[2]+ " , "+track_roi[3]+ " , "+track_roi[4]);
			SmartNative.setROIData(track_roi, 1);
			track_roi[0] = GlobalParams.TRACKING_STATUS_IDLE;
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
	
	String[] ISPtemp = null;
	private static float[] ispData =  new float[13];
	private static String[] ISPData = null;
	//  ISP data  , for zoom and x2pro
//    class ISPDataCallback implements Q3aDataCallback {
//    	
//    	
//        @Override
//        public void onQ3aData(byte[] data, Camera camera) {
//        	
//    		if(null != data){
//    			String cameraISP = new String(data);
//    			ISPtemp = cameraISP.split(";");
//    		}
//        	
//    		if(null != ISPtemp){
//    			for(int i = 1; i <ISPtemp.length && i < 13; i++){
//    				ISPData = ISPtemp[i].split("=");
//                    if (null == ISPData || null == ISPData[1]) {
//                        break;
//                    }
//    				ispData[i] = Float.parseFloat(ISPData[1]);			
//    			}
//    		}	
////    		IVCSceneNative.setISPData(ispData, ispData.length);
//    		SmartNative.setISPData(ispData, ispData.length);
//        	String cameraISP = new String(data);
//        	android.util.Log.i("CameraISP", "ISP data:" + cameraISP);
//        }
//    }

}
