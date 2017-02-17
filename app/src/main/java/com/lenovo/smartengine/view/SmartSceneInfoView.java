package com.lenovo.smartengine.view;

import com.lenovo.smartengine.jni.SmartNative;
import com.lenovo.smartengine.util.GlobalParams;

import android.content.Context;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.DashPathEffect;
import android.graphics.Paint;
import android.graphics.Path;
import android.graphics.PathEffect;
import android.graphics.Typeface;
import android.graphics.Paint.Style;
import android.util.AttributeSet;
import android.util.Log;
import android.view.View;

public class SmartSceneInfoView extends View {

    public static final int  FLAG_DETECTION_ERROR = -1;
    public static final int  FLAG_DETECTION_NORMAL = 0;
    public static final int  FLAG_DETECTION_PORTRAIT = 1;
    public static final int  FLAG_DETECTION_LOWLIT = 2;
    public static final int  FLAG_DETECTION_ULTRA_LOWLIT = 3;
    public static final int  FLAG_DETECTION_PORTRAIT_BACKLIT = 4;
    public static final int  FLAG_DETECTION_PORTRAIT_LOWLIT = 5;
    public static final int  FLAG_DETECTION_BACKLIGHT = 6;
    public static final int  FLAG_DETECTION_LANDSCAPE = 7;
    public static final int  FLAG_DETECTION_FOOD = 8;
    public static final int  FLAG_DETECTION_QRCODE = 9;
    public static final int  FLAG_DETECTION_DOCUMENT = 10;
    public static final int  FLAG_DETECTION_JITTERING = 11;
    public static final int  FLAG_DETECTION_MOVE = 12;    
    public static final int  FLAG_GUIDE_ALLOW = 1;
    
    Paint paint_Result = new Paint();
    Paint paint_face = new Paint();
    Paint paint_tracking = new Paint();
    private int mSceneFlag = -1;
    private int mSceneMotionFlag = -1;
	
	public SmartSceneInfoView(Context context, AttributeSet attrs) {
		super(context, attrs);
		// TODO Auto-generated constructor stub
		
		paint_Result.setStyle(Style.FILL);
		paint_Result.setTextSize(40);
		paint_Result.setTypeface(Typeface.DEFAULT_BOLD);
		paint_Result.setColor(Color.argb(255, 255, 255, 255));
		
		paint_face.setStyle(Style.STROKE);
		paint_face.setColor(Color.argb(125, 255, 255, 0));  
		paint_face.setStrokeWidth(10); 
		
		paint_tracking.setStyle(Style.STROKE);
		paint_tracking.setColor(Color.argb(125, 0, 255, 0));  
		paint_tracking.setStrokeWidth(3);
	}

	public void setSceneFlag(int sceneFlag, int sceneMotionFlag)
	{
		mSceneFlag = sceneFlag;
		mSceneMotionFlag = sceneMotionFlag;
		Log.i("SetSceneFlagTag", "Set Flag test 1, Scene: " + mSceneFlag
				+ " , motion: " + mSceneMotionFlag);
	}
	@Override
	protected void onDraw(Canvas canvas) {
		// TODO Auto-generated method stub
		
		int[] faces = CameraSurfaceView.faceDrawData;
		
		//draw face
		//for(int i = 0; i< CameraSurfaceView.faceLength; i ++){
		//	canvas.drawRect(faces[4*i+0], faces[4*i+1], faces[4*i+2], faces[4*i+3], paint_face);
		//}

		paint_r.setStyle(Style.FILL);
		paint_r.setColor(Color.argb(95, 255, 0, 0));  
		paint_r.setStrokeWidth(15); 
		
		paint_g.setStyle(Style.FILL);
		paint_g.setColor(Color.argb(95, 0, 255, 0));  
		paint_g.setStrokeWidth(15); 
		
		paint_b.setStyle(Style.FILL);
		paint_b.setColor(Color.argb(95, 0, 0, 255));  
		paint_b.setStrokeWidth(15); 
		
		paint_y.setStyle(Style.FILL);
		paint_y.setColor(Color.argb(95, 255, 255, 0));  
		paint_y.setStrokeWidth(15); 

		paint_target.setStyle(Style.STROKE);
		paint_target.setColor(Color.argb(80, 20, 150, 250));  //浅蓝
		paint_target.setStrokeWidth(5); 

		paint_recomm.setStyle(Style.STROKE);
		paint_recomm.setColor(Color.argb(120, 20, 150, 250));  //浅蓝
		paint_recomm.setStrokeWidth(15); 

		paint_white.setStyle(Style.STROKE);
		paint_white.setColor(Color.argb(60, 255, 255, 255));  //白色
		paint_white.setStrokeWidth(15); 

		paint_white_target.setStyle(Style.FILL_AND_STROKE);
		paint_white_target.setColor(Color.argb(35, 255, 255, 255));  
		paint_white_target.setStrokeWidth(15); 

		paint_info.setStyle(Style.FILL);
		paint_info.setTextSize(40);
		paint_info.setTypeface(Typeface.DEFAULT_BOLD);
		paint_info.setColor(Color.argb(255, 255, 255, 255)); 
		
		paint_zoom.setTextSize(250);
		paint_zoom.setColor(Color.argb(95, 251, 254, 1));
		paint_zoom.setStrokeWidth(20); 		
		
		int ori = GlobalParams.orientation;
		
		int preWidth = CameraSurfaceView.previewWidth;
		int preHeight = CameraSurfaceView.previewHeight;
		int screenWidth = GlobalParams.SCREEN_WIDTH;
		int screenHeight = GlobalParams.SCREEN_HEIGHT;
//		float ratioW1 = (float)screenHeight / preWidth;
//		float ratioH1 = (float)screenWidth / preHeight;
		float ratioW1 = (float)screenWidth / preWidth;
		float ratioH1 = (float)screenHeight / preHeight;
		Log.i("ScreenSizeLog", "---->Screen width*Height: " + screenWidth + "*"  + screenHeight + "  Ori: "+ori +"---->preview width*Height: " + preWidth + "*"  + preHeight);

		float ratioW = 1.0f;
		float ratioH = 1.0f;
		
	//	Log.v("poGuide", "screenWidth="+screenWidth+" , screenHeight= "+screenHeight);
		
		if (ori == 270 || ori == 90) {
			if (ori == 270) {
				
				faceTarget[0] = screenWidth  /3;
				faceTarget[1] = screenHeight /3;
				faceTarget[2] = screenWidth * 2/3;
				faceTarget[3] = screenHeight /3;
							
			} else {
				
				faceTarget[0] = screenWidth * 2/3;
				faceTarget[1] = screenHeight * 2/3;
				faceTarget[2] = screenWidth /3;
				faceTarget[3] = screenHeight * 2/3;
			}
		}else{
			if (ori == 0) {		
				faceTarget[4] = screenWidth /3;
				faceTarget[5] = screenHeight /2;
			} else {			
				faceTarget[4] = screenWidth * 2/3;
				faceTarget[5] = screenHeight /2;
			}
		}
		
		int[] SmartData = SmartNative.getCompositeResult();
		float[] qrCodePos = SmartNative.getQRPostion();
//		float[] qrCodePosTemp = SmartNative.getQRPostion();
//		float[] qrCodePos = new float[8];
		boolean isLanguageCN = true;

		
		if(CameraSurfaceView.track_roi[0] == 1 ){
			canvas.drawRect(CameraSurfaceView.track_roi[1],CameraSurfaceView.track_roi[2],CameraSurfaceView.track_roi[3],CameraSurfaceView.track_roi[4],paint_tracking);
		}
//		Log.v("tldTrackingRes", "["+CameraSurfaceView.track_roi[0]+" ,"+CameraSurfaceView.track_roi[1] + ","+ CameraSurfaceView.track_roi[2]+","+CameraSurfaceView.track_roi[3]+","+CameraSurfaceView.track_roi[4]+"]");
		if(SmartData[20] == 1 ){
			canvas.drawRect(SmartData[21]*ratioW1, SmartData[22]*ratioH1, SmartData[23]*ratioW1, SmartData[24]*ratioH1, paint_tracking);
			canvas.drawText("pEx:"+SmartData[25], SmartData[21]*ratioW1, SmartData[22]*ratioH1, paint_info);
		}
//		Log.v("tldTrackingRes", "["+SmartData[21]*ratioW1+" ,"+SmartData[22]*ratioH1+","+SmartData[23]*ratioW1+","+SmartData[24]*ratioH1);
		
		if (SmartData[0] == FLAG_DETECTION_QRCODE){
//			qrCodePos[0] = (1 - qrCodePosTemp[1]/preHeight) * screenHeight;
//			qrCodePos[1] = qrCodePosTemp[0]/preWidth * screenWidth;
//			qrCodePos[2] = (1 - qrCodePosTemp[3]/preHeight) * screenHeight;
//			qrCodePos[3] = qrCodePosTemp[2]/preWidth * screenWidth;
//			qrCodePos[4] = (1 - qrCodePosTemp[5]/preHeight) * screenHeight;
//			qrCodePos[5] = qrCodePosTemp[4]/preWidth * screenWidth;
//			qrCodePos[6] = (1 - qrCodePosTemp[7]/preHeight) * screenHeight;
//			qrCodePos[7] = qrCodePosTemp[6]/preWidth * screenWidth;
			for(int i = 0; i < qrCodePos.length; i++){
				qrCodePos[i] = qrCodePos[i] * ratioW1;
			}
			
			canvas.drawLine(qrCodePos[0], qrCodePos[1], qrCodePos[2], qrCodePos[3] ,paint_r);
			canvas.drawLine(qrCodePos[2], qrCodePos[3], qrCodePos[4], qrCodePos[5] ,paint_g);
			canvas.drawLine(qrCodePos[4], qrCodePos[5], qrCodePos[6], qrCodePos[7] ,paint_b);
			canvas.drawLine(qrCodePos[6], qrCodePos[7], qrCodePos[0], qrCodePos[1] ,paint_y);
		} 	
		
		if(isLanguageCN && GlobalParams.OPEN_ICG){
			//---------------portrait recommendation------------------------------------
			if(SmartData[0] == FLAG_DETECTION_PORTRAIT && SmartData[1] == FLAG_GUIDE_ALLOW){
				
				paint_target.setStyle(Style.STROKE);
				paint_target.setColor(Color.argb(80, 20, 150, 250));  //浅蓝
				paint_target.setStrokeWidth(5); 		

				paint_white.setStyle(Style.STROKE);
				paint_white.setColor(Color.argb(60, 255, 255, 255));  //白色
				paint_white.setStrokeWidth(15); 

				paint_white_target.setStyle(Style.FILL_AND_STROKE);
				paint_white_target.setColor(Color.argb(35, 255, 255, 255));  
				paint_white_target.setStrokeWidth(15); 

				paint_info.setStyle(Style.FILL);
				paint_info.setTextSize(40);
				paint_info.setTypeface(Typeface.DEFAULT_BOLD);
				paint_info.setColor(Color.argb(255, 255, 255, 255)); 

				String portraitRatio = SmartData[2] + "%";
				int ratio = 0;
				
				if(SmartData[3] == 0){
					//放大
					if(ori == 0 || ori == 180){
						ratio = (int)(2.5 * SmartData[2]);
						paint_target.setStrokeWidth(5);
						guideDrawCircle((int)(SmartData[4] * ratioH),
										(int)(SmartData[5] * ratioW), 250, canvas, paint_target);
						guideDrawCircle((int)(SmartData[4] * ratioH),
										(int)(SmartData[5] * ratioW), 280, canvas, paint_target);			
					}else{
						ratio = (int)(1.5 * SmartData[2]);
						paint_target.setStrokeWidth(5);
						guideDrawCircle((int)(SmartData[4] * ratioH),
										(int)(SmartData[5] * ratioW), 150, canvas, paint_target);
						guideDrawCircle((int)(SmartData[4] * ratioH),
										(int)(SmartData[5] * ratioW), 165, canvas, paint_target);			
					}
					guideDrawCircle((int)(SmartData[4] * ratioH),
									(int)(SmartData[5] * ratioW), ratio, canvas, paint_white_target);
					guideDrawText((int)(SmartData[4] * ratioH),
								  (int)(SmartData[5] * ratioW), ori, portraitRatio, canvas, paint_info);
					guideDrawTag((int)(SmartData[4] * ratioH),
								 (int)(SmartData[5] * ratioW), 40, ori,  "离近一点", canvas, paint_info);
				}else if(SmartData[3] == 2){
					//缩小
					paint_target.setColor(Color.argb(80, 250, 80, 130)); //pink color
					if(ori == 0 || ori == 180){
						paint_white.setStrokeWidth(5);
						guideDrawCircle((int)(SmartData[4] * ratioH),
										(int)(SmartData[5] * ratioW), 250, canvas, paint_target);
						guideDrawCircle((int)(SmartData[4] * ratioH),
										(int)(SmartData[5] * ratioW), 280, canvas, paint_target);			
					}else{
						paint_white.setStrokeWidth(5);
						guideDrawCircle((int)(SmartData[4] * ratioH),
										(int)(SmartData[5] * ratioW), 150, canvas, paint_target);
						guideDrawCircle((int)(SmartData[4] * ratioH),
										(int)(SmartData[5] * ratioW), 165, canvas, paint_target);			
					}
					guideDrawText((int)(SmartData[4] * ratioH),
							      (int)(SmartData[5] * ratioW), ori, "100%", canvas, paint_info);
					guideDrawTag((int)(SmartData[4] * ratioH),
							     (int)(SmartData[5] * ratioW), 40, ori,  "离远一点", canvas, paint_info);
				}else{
					//合适大小
					if(SmartData[2] >= 100){
						portraitRatio = "100%";
					}
					
					if(ori == 90 || ori == 270){
						//横屏正脸
						
						Log.v("poGuide","90270:"+ faceTarget[0] +" , "+ faceTarget[1] + " , "+ faceTarget[2]+" , "+faceTarget[3]+" , l="+SmartData[8]+" , r="+SmartData[9]);
						
						if(SmartData[7] == 2){
							//距离 perfect
							paint_white.setColor(Color.argb(80, 20, 150, 250));  //blue
							paint_white.setStrokeWidth(15);
							guideDrawCircle((int)(faceTarget[0] * ratioH), (int)(faceTarget[1] * ratioW),  150, canvas, paint_white, SmartData[8]);
							guideDrawCircle((int)(faceTarget[2] * ratioH), (int)(faceTarget[3] * ratioW), 150, canvas, paint_white, SmartData[9]);	
							guideDrawText((int)(SmartData[4] * ratioH),
								  	  	  (int)(SmartData[5] * ratioW), ori, portraitRatio, canvas, paint_info);
							guideDrawTag((int)(SmartData[4] * ratioH),
							         	 (int)(SmartData[5] * ratioW), 80, ori,  "Prefect", canvas, paint_info);
						}else if(SmartData[7] == 1){
							//距离 good
							paint_white.setColor(Color.argb(120, 255, 255, 255));  //白色
							paint_white.setStrokeWidth(15);
							guideDrawCircle((int)(faceTarget[0] * ratioH), (int)(faceTarget[1] * ratioW),  150, canvas, paint_white, SmartData[8]);
							guideDrawCircle((int)(faceTarget[2] * ratioH), (int)(faceTarget[3] * ratioW), 150, canvas, paint_white, SmartData[9]);
							paint_target.setColor(Color.argb(80, 20, 150, 250));  //blue color
							guideDrawCircle((int)(SmartData[4] * ratioH),
											(int)(SmartData[5] * ratioW), 150, canvas, paint_target);
							guideDrawCircle((int)(SmartData[4] * ratioH),
											(int)(SmartData[5] * ratioW), 165, canvas, paint_target);
							guideDrawText((int)(SmartData[4] * ratioH),
									  	  (int)(SmartData[5] * ratioW), ori, portraitRatio, canvas, paint_info);
							guideDrawTag((int)(SmartData[4] * ratioH),
						         	 	 (int)(SmartData[5] * ratioW), 80, ori,  "Good", canvas, paint_info);
						}else{
							//距离不达标
							paint_white.setColor(Color.argb(120, 255, 255, 255));  //白色
							paint_white.setStrokeWidth(15);
							guideDrawCircle((int)(faceTarget[0] * ratioH), (int)(faceTarget[1] * ratioW),  150, canvas, paint_white, SmartData[8]);
							guideDrawCircle((int)(faceTarget[2] * ratioH), (int)(faceTarget[3] * ratioW), 150, canvas, paint_white, SmartData[9]);
							paint_target.setColor(Color.argb(80, 20, 150, 250));  //blue color
							guideDrawCircle((int)(SmartData[4] * ratioH),
											(int)(SmartData[5] * ratioW), 150, canvas, paint_target);
							guideDrawCircle((int)(SmartData[4] * ratioH),
											(int)(SmartData[5] * ratioW), 165, canvas, paint_target);
							guideDrawText((int)(SmartData[4] * ratioH),
									  	  (int)(SmartData[5] * ratioW), ori, portraitRatio, canvas, paint_info);
						}	
					}else{
						//竖屏正脸
						Log.v("poGuide","90270:"+ faceTarget[4] +" , "+ faceTarget[5] + " l="+SmartData[8]+" , r="+SmartData[9]);
						if(SmartData[7] == 2){
							//距离perfect
							paint_white.setColor(Color.argb(80, 20, 150, 250));  //blue
							paint_white.setStrokeWidth(30);
							guideDrawCircle((int)(faceTarget[4] * ratioH), (int)(faceTarget[5] * ratioW),  265, canvas, paint_white, 100);	
							guideDrawText((int)(SmartData[4] * ratioH),
								  	  	  (int)(SmartData[5] * ratioW), ori, portraitRatio, canvas, paint_info);
							guideDrawTag((int)(SmartData[4] * ratioH),
										 (int)(SmartData[5] * ratioW), 80, ori,  "Prefect", canvas, paint_info);
						}else if(SmartData[7] == 1){
							//距离 good
							paint_white.setColor(Color.argb(120, 255, 255, 255));  //白色
							paint_white.setStrokeWidth(30);
							guideDrawCircle((int)(faceTarget[4] * ratioH), (int)(faceTarget[5] * ratioW),  265, canvas, paint_white, 100);
							paint_target.setColor(Color.argb(80, 20, 150, 250));  //blue color
							guideDrawCircle((int)(SmartData[4] * ratioH),
											(int)(SmartData[5] * ratioW), 250, canvas, paint_target);
							guideDrawCircle((int)(SmartData[4] * ratioH),
											(int)(SmartData[5] * ratioW), 280, canvas, paint_target);
							guideDrawText((int)(SmartData[4] * ratioH),
									  	  (int)(SmartData[5] * ratioW), ori, portraitRatio, canvas, paint_info);
							guideDrawTag((int)(SmartData[4] * ratioH),
						         	 	 (int)(SmartData[5] * ratioW), 80, ori,  "Good", canvas, paint_info);
						}else{
							//距离不达标
							paint_white.setColor(Color.argb(60, 255, 255, 255));  //白色
							paint_white.setStrokeWidth(30);
							guideDrawCircle((int)(faceTarget[4] * ratioH), (int)(faceTarget[5] * ratioW),  265, canvas, paint_white, 100);
							paint_target.setColor(Color.argb(80, 20, 150, 250));  //blue color
							guideDrawCircle((int)(SmartData[4] * ratioH),
											(int)(SmartData[5] * ratioW), 250, canvas, paint_target);
							guideDrawCircle((int)(SmartData[4] * ratioH),
											(int)(SmartData[5] * ratioW), 280, canvas, paint_target);
							guideDrawText((int)(SmartData[4] * ratioH),
									  	  (int)(SmartData[5] * ratioW), ori, portraitRatio, canvas, paint_info);
						}	
					}
				}	
			}else{
			}
			
			//---------------ellipse recommendation------------------------------------				
			if (SmartData[0] == FLAG_DETECTION_FOOD && SmartData[1] == FLAG_GUIDE_ALLOW){
				paint_target.setStyle(Style.STROKE);
				paint_target.setColor(Color.argb(80, 20, 150, 250));  //浅蓝
				paint_target.setStrokeWidth(5); 		

				paint_white.setStyle(Style.STROKE);
				paint_white.setColor(Color.argb(60, 255, 255, 255));  //白色
				paint_white.setStrokeWidth(15); 

				paint_white_target.setStyle(Style.FILL_AND_STROKE);
				paint_white_target.setColor(Color.argb(35, 255, 255, 255));  
				paint_white_target.setStrokeWidth(15); 

				paint_info.setStyle(Style.FILL);
				paint_info.setTextSize(40);
				paint_info.setTypeface(Typeface.DEFAULT_BOLD);
				paint_info.setColor(Color.argb(255, 255, 255, 255)); 
				
				int x0 = GlobalParams.SCREEN_HEIGHT/2 ;
				int y0 = GlobalParams.SCREEN_WIDTH /2 ;
				String foodRatio = SmartData[2] + "%";
				if (SmartData[7] != 0) {
					// ready Grab				
					if(SmartData[7] == 2){
						// perfect 判定 
//						guideDrawCircle((int)(SmartData[4] * ratioH),
//										(int)(SmartData[5] * ratioW), 150, canvas, paint_target);
//						guideDrawCircle((int)(SmartData[12]  * ratioH),
//										(int)(SmartData[13] * ratioW), 150, canvas, paint_white);
						paint_white.setColor(Color.argb(80, 20, 150, 250));
						paint_white.setStrokeWidth(15);
						guideDrawCircle((int)(SmartData[12] * ratioH),
								        (int)(SmartData[13] * ratioW), 150, canvas, paint_white);
						guideDrawText((int)(SmartData[4] * ratioH),
							          (int)(SmartData[5] * ratioW), ori, foodRatio, canvas, paint_info);
						guideDrawTag((int)(SmartData[4] * ratioH),
							         (int)(SmartData[5] * ratioW), 80, ori,  "Prefect", canvas, paint_info);
//						guideDrawText((int)(SmartData[4] * ratioH),
//								      (int)(SmartData[5] * ratioW + 80), ori,  "Prefect", canvas, paint_info);
					}else{
						// good 判定
						paint_target.setColor(Color.argb(80, 20, 150, 250));  //blue color
						guideDrawCircle((int)(SmartData[4] * ratioH),
										(int)(SmartData[5] * ratioW), 150, canvas, paint_target);
						guideDrawCircle((int)(SmartData[4] * ratioH),
										(int)(SmartData[5] * ratioW), 165, canvas, paint_target);
						
						guideDrawCircle((int)(SmartData[12] * ratioH),
										(int)(SmartData[13] * ratioW), 150, canvas, paint_white);
						guideDrawText((int)(SmartData[4] * ratioH),
								      (int)(SmartData[5] * ratioW), ori, foodRatio, canvas, paint_info);
						guideDrawTag((int)(SmartData[4] * ratioH),
						             (int)(SmartData[5] * ratioW), 80, ori,  "Good", canvas, paint_info);
//						guideDrawText((int)(SmartData[4] * ratioH),
//								      (int)(SmartData[5] * ratioW + 80), ori, "Good", canvas, paint_info);
					}
				} else {
					// not ready Grab

					if(SmartData[3] == 0){
						//放大
						int ratio = (int)(1.5 * SmartData[2]);
						guideDrawCircle((int)(SmartData[4] * ratioH),
										(int)(SmartData[5] * ratioW), 150, canvas, paint_white);
						guideDrawCircle((int)(SmartData[4] * ratioH),
										(int)(SmartData[5] * ratioW), ratio, canvas, paint_white_target);
						guideDrawText((int)(SmartData[4] * ratioH),
									  (int)(SmartData[5] * ratioW), ori, foodRatio, canvas, paint_info);
						guideDrawTag((int)(SmartData[4] * ratioH),
								(int)(SmartData[5] * ratioW), 40, ori,  "离近一点", canvas, paint_info);
					}else if(SmartData[3] == 2){
						//缩小
						paint_target.setColor(Color.argb(80, 250, 80, 130)); //pink color
						guideDrawCircle((int)(SmartData[4] * ratioH),
										(int)(SmartData[5] * ratioW), 150, canvas, paint_target);
						guideDrawCircle((int)(SmartData[4] * ratioH),
										(int)(SmartData[5] * ratioW), 165, canvas, paint_target);
						guideDrawText((int)(SmartData[4] * ratioH),
								      (int)(SmartData[5] * ratioW), ori, "100%", canvas, paint_info);
						guideDrawTag((int)(SmartData[4] * ratioH),
								(int)(SmartData[5] * ratioW), 40, ori,  "离远一点", canvas, paint_info);
					}else{
						//合适大小
						paint_target.setColor(Color.argb(80, 20, 150, 250));  //blue color
						guideDrawCircle((int)(SmartData[4] * ratioH),
										(int)(SmartData[5] * ratioW), 150, canvas, paint_target);
						guideDrawCircle((int)(SmartData[4] * ratioH),
										(int)(SmartData[5] * ratioW), 165, canvas, paint_target);
						guideDrawCircle((int)(SmartData[12] * ratioH),
										(int)(SmartData[13] * ratioW), 150, canvas, paint_white);
						guideDrawText((int)(SmartData[4] * ratioH),
								  	  (int)(SmartData[5] * ratioW), ori, foodRatio, canvas, paint_info);
					}
				}
			} else {	
			}
			
//			//---------------line recommendation------------------------------------
//			//-------------draw Line---------------------------------------------			
			if (SmartData[0] == FLAG_DETECTION_LANDSCAPE && SmartData[1] == FLAG_GUIDE_ALLOW){
				
				paint_target.setStyle(Style.STROKE);
				paint_target.setColor(Color.argb(80, 20, 150, 250));  //浅蓝
				paint_target.setStrokeWidth(5); 	
				
				paint_white.setStyle(Style.STROKE);
				paint_white.setColor(Color.argb(60, 255, 255, 255));  //白色
				paint_white.setStrokeWidth(15);
				
				paint_info.setStyle(Style.FILL);
				paint_info.setTextSize(40);
				paint_info.setTypeface(Typeface.DEFAULT_BOLD);
				paint_info.setColor(Color.argb(255, 255, 255, 255));
				
				String landAngle = SmartData[2] + "°";
						
				if (SmartData[7] != 0) {
					// ready Grab
					
					if(SmartData[7] == 1){
						// Good
						paint_target.setColor(Color.argb(100, 20, 150, 250));  //浅蓝
						paint_target.setStrokeWidth(4);					
						paint_white.setColor(Color.argb(40, 255, 255, 255));  //白色	
						switch(SmartData[6]){
						case 0:
//							angleTracker(GlobalParams.SCREEN_WIDTH*1/2, GlobalParams.SCREEN_HEIGHT*2/3, canvas, paint_white, paint_target); 	// for vertical
//							guideDrawText(GlobalParams.SCREEN_WIDTH*1/2, GlobalParams.SCREEN_HEIGHT*2/3, ori, landAngle, canvas, paint_info);	// for vertical
//							guideDrawTag(GlobalParams.SCREEN_WIDTH*1/2, GlobalParams.SCREEN_HEIGHT*2/3, 80, ori,  "Good", canvas, paint_info);	// for vertical				
//							canvas.drawLine(0, GlobalParams.SCREEN_HEIGHT*2/3, GlobalParams.SCREEN_WIDTH, GlobalParams.SCREEN_HEIGHT*2/3, paint_target);	// for vertical
							
							angleTracker(GlobalParams.SCREEN_WIDTH*2/3, GlobalParams.SCREEN_HEIGHT /2, canvas, paint_white, paint_target);
							guideDrawTextForHorizontal(GlobalParams.SCREEN_WIDTH*2/3, GlobalParams.SCREEN_HEIGHT /2, ori, landAngle, canvas, paint_info);
							guideDrawTagForHorizontal( GlobalParams.SCREEN_WIDTH*2/3, GlobalParams.SCREEN_HEIGHT /2, 80, ori,  "Good", canvas, paint_info);
							canvas.drawLine( GlobalParams.SCREEN_WIDTH*2/3 , 0 , GlobalParams.SCREEN_WIDTH*2/3 , GlobalParams.SCREEN_HEIGHT , paint_target);
							
							break;
						case 90:		
//							angleTracker(GlobalParams.SCREEN_WIDTH*2/3, GlobalParams.SCREEN_HEIGHT*1/2, canvas, paint_white, paint_target);
//							guideDrawText(GlobalParams.SCREEN_WIDTH*2/3, GlobalParams.SCREEN_HEIGHT*1/2, ori, landAngle, canvas, paint_info);
//							guideDrawTag(GlobalParams.SCREEN_WIDTH*2/3, GlobalParams.SCREEN_HEIGHT*1/2, 80, ori,  "Good", canvas, paint_info);					
//							canvas.drawLine(GlobalParams.SCREEN_WIDTH*2/3, 0, GlobalParams.SCREEN_WIDTH*2/3, GlobalParams.SCREEN_HEIGHT, paint_target);
							
							angleTracker(GlobalParams.SCREEN_WIDTH*2/3, GlobalParams.SCREEN_HEIGHT /3, canvas, paint_white, paint_target);
							guideDrawTextForHorizontal(GlobalParams.SCREEN_WIDTH*2/3,  GlobalParams.SCREEN_HEIGHT /3, ori, landAngle, canvas, paint_info);
							guideDrawTagForHorizontal( GlobalParams.SCREEN_WIDTH*2/3,  GlobalParams.SCREEN_HEIGHT /3, 80, ori,  "Good", canvas, paint_info);
							canvas.drawLine(0,GlobalParams.SCREEN_HEIGHT/3 ,GlobalParams.SCREEN_WIDTH, GlobalParams.SCREEN_HEIGHT/3, paint_target);
							
							break;
						case 180:
//							angleTracker(GlobalParams.SCREEN_WIDTH*1/2, GlobalParams.SCREEN_HEIGHT*1/3, canvas, paint_white, paint_target);
//							guideDrawText(GlobalParams.SCREEN_WIDTH*1/2, GlobalParams.SCREEN_HEIGHT*1/3, ori, landAngle, canvas, paint_info);
//							guideDrawTag(GlobalParams.SCREEN_WIDTH*1/2, GlobalParams.SCREEN_HEIGHT*1/3, 80, ori,  "Good", canvas, paint_info);
//							canvas.drawLine(0, GlobalParams.SCREEN_HEIGHT*1/3, GlobalParams.SCREEN_WIDTH, GlobalParams.SCREEN_HEIGHT*1/3, paint_target);							

							break;
						case 270:
//							angleTracker(GlobalParams.SCREEN_WIDTH*1/3, GlobalParams.SCREEN_HEIGHT*1/2, canvas, paint_white, paint_target);				// for vertical
//							guideDrawText(GlobalParams.SCREEN_WIDTH*1/3, GlobalParams.SCREEN_HEIGHT*1/2, ori, landAngle, canvas, paint_info);			// for vertical
//							guideDrawTag(GlobalParams.SCREEN_WIDTH*1/3, GlobalParams.SCREEN_HEIGHT*1/2, 80, ori,  "Good", canvas, paint_info);			// for vertical
//							canvas.drawLine(GlobalParams.SCREEN_WIDTH*1/3, 0, GlobalParams.SCREEN_WIDTH*1/3, GlobalParams.SCREEN_HEIGHT, paint_target);	// for vertical							
							angleTracker(GlobalParams.SCREEN_WIDTH*1/2, GlobalParams.SCREEN_HEIGHT*2/3, canvas, paint_white, paint_target);
							guideDrawTextForHorizontal(GlobalParams.SCREEN_WIDTH*1/2, GlobalParams.SCREEN_HEIGHT*2/3, ori, landAngle, canvas, paint_info);
							guideDrawTagForHorizontal( GlobalParams.SCREEN_WIDTH*1/2, GlobalParams.SCREEN_HEIGHT*2/3, 80, ori,  "Good", canvas, paint_info);
							canvas.drawLine(0,GlobalParams.SCREEN_HEIGHT*2/3 ,GlobalParams.SCREEN_WIDTH, GlobalParams.SCREEN_HEIGHT*2/3, paint_target);
							break;
						default:
							break;				
						}
					}else{
						// Perfect
						paint_target.setColor(Color.argb(100, 20, 150, 250));  //浅蓝   //20, 150, 250
						paint_target.setStrokeWidth(4);					
						paint_white.setColor(Color.argb(40, 20, 150, 250));  //浅蓝	
						switch(SmartData[6]){
						case 0:
//							angleTracker(GlobalParams.SCREEN_WIDTH*1/2, GlobalParams.SCREEN_HEIGHT*2/3, canvas, paint_white, paint_target);					//for vertical
//							guideDrawText(GlobalParams.SCREEN_WIDTH*1/2, GlobalParams.SCREEN_HEIGHT*2/3, ori, landAngle, canvas, paint_info);				//for vertical
//							guideDrawTag(GlobalParams.SCREEN_WIDTH*1/2, GlobalParams.SCREEN_HEIGHT*2/3, 80, ori,  "Perfect", canvas, paint_info);			//for vertical				
//							canvas.drawLine(0, GlobalParams.SCREEN_HEIGHT*2/3, GlobalParams.SCREEN_WIDTH, GlobalParams.SCREEN_HEIGHT*2/3, paint_target);	//for vertical
							
							angleTracker(GlobalParams.SCREEN_WIDTH*2/3, GlobalParams.SCREEN_HEIGHT /2, canvas, paint_white, paint_target);
							guideDrawTextForHorizontal(GlobalParams.SCREEN_WIDTH*2/3, GlobalParams.SCREEN_HEIGHT /2, ori, landAngle, canvas, paint_info);
							guideDrawTagForHorizontal( GlobalParams.SCREEN_WIDTH*2/3, GlobalParams.SCREEN_HEIGHT /2, 80, ori,  "Perfect", canvas, paint_info);
							canvas.drawLine( GlobalParams.SCREEN_WIDTH*2/3 , 0 , GlobalParams.SCREEN_WIDTH*2/3 , GlobalParams.SCREEN_HEIGHT , paint_target);
							break;
						case 90:		
//							angleTracker(GlobalParams.SCREEN_WIDTH*2/3, GlobalParams.SCREEN_HEIGHT*1/2, canvas, paint_white, paint_target);
//							guideDrawText(GlobalParams.SCREEN_WIDTH*2/3, GlobalParams.SCREEN_HEIGHT*1/2, ori, landAngle, canvas, paint_info);
//							guideDrawTag(GlobalParams.SCREEN_WIDTH*2/3, GlobalParams.SCREEN_HEIGHT*1/2, 80, ori,  "Perfect", canvas, paint_info);					
//							canvas.drawLine(GlobalParams.SCREEN_WIDTH*2/3, 0, GlobalParams.SCREEN_WIDTH*2/3, GlobalParams.SCREEN_HEIGHT, paint_target);
							angleTracker(GlobalParams.SCREEN_WIDTH*1/2, GlobalParams.SCREEN_HEIGHT/3, canvas, paint_white, paint_target);
							guideDrawTextForHorizontal(GlobalParams.SCREEN_WIDTH*1/2, GlobalParams.SCREEN_HEIGHT/3, ori, landAngle, canvas, paint_info);
							guideDrawTagForHorizontal( GlobalParams.SCREEN_WIDTH*1/2, GlobalParams.SCREEN_HEIGHT/3, 80, ori,  "Perfect", canvas, paint_info);
							canvas.drawLine(0,GlobalParams.SCREEN_HEIGHT/3 ,GlobalParams.SCREEN_WIDTH, GlobalParams.SCREEN_HEIGHT/3, paint_target);
							break;
						case 180:
//							angleTracker(GlobalParams.SCREEN_WIDTH*1/2, GlobalParams.SCREEN_HEIGHT*1/3, canvas, paint_white, paint_target);
//							guideDrawText(GlobalParams.SCREEN_WIDTH*1/2, GlobalParams.SCREEN_HEIGHT*1/3, ori, landAngle, canvas, paint_info);
//							guideDrawTag(GlobalParams.SCREEN_WIDTH*1/2, GlobalParams.SCREEN_HEIGHT*1/3, 80, ori,  "Perfect", canvas, paint_info);
//							canvas.drawLine(0, GlobalParams.SCREEN_HEIGHT*1/3, GlobalParams.SCREEN_WIDTH, GlobalParams.SCREEN_HEIGHT*1/3, paint_target);

							break;
						case 270:
//							angleTracker(GlobalParams.SCREEN_WIDTH*1/3, GlobalParams.SCREEN_HEIGHT*1/2, canvas, paint_white, paint_target);				// for vertical
//							guideDrawText(GlobalParams.SCREEN_WIDTH*1/3, GlobalParams.SCREEN_HEIGHT*1/2, ori, landAngle, canvas, paint_info);			// for vertical
//							guideDrawTag(GlobalParams.SCREEN_WIDTH*1/3, GlobalParams.SCREEN_HEIGHT*1/2, 80, ori,  "Perfect", canvas, paint_info);		// for vertical
//							canvas.drawLine(GlobalParams.SCREEN_WIDTH*1/3, 0, GlobalParams.SCREEN_WIDTH*1/3, GlobalParams.SCREEN_HEIGHT, paint_target);	// for vertical						
							angleTracker(GlobalParams.SCREEN_WIDTH*1/2, GlobalParams.SCREEN_HEIGHT*2/3, canvas, paint_white, paint_target);
							guideDrawTextForHorizontal(GlobalParams.SCREEN_WIDTH*1/2, GlobalParams.SCREEN_HEIGHT*2/3, ori, landAngle, canvas, paint_info);
							guideDrawTagForHorizontal( GlobalParams.SCREEN_WIDTH*1/2, GlobalParams.SCREEN_HEIGHT*2/3, 80, ori,  "Perfect", canvas, paint_info);
							canvas.drawLine(0,GlobalParams.SCREEN_HEIGHT*2/3 ,GlobalParams.SCREEN_WIDTH, GlobalParams.SCREEN_HEIGHT*2/3, paint_target);
							break;
						default:
							break;				
						}
					}
				} else {
					// not ready Grab			
					paint_target.setColor(Color.argb(100, 255, 255, 255));  //白色
					paint_target.setStrokeWidth(4);					
					paint_white.setColor(Color.argb(40, 255, 255, 255));    //白色
					if(Math.abs(SmartData[2]) <= 6){
						paint_recomm.setColor(Color.argb(80, 255, 0, 0)); // red color
						switch(SmartData[6]){
						case 0:
//							angleTracker(GlobalParams.SCREEN_WIDTH*1/2, GlobalParams.SCREEN_HEIGHT*2/3, canvas, paint_white, paint_target);					//for vertical
//							guideDrawText(GlobalParams.SCREEN_WIDTH*1/2, GlobalParams.SCREEN_HEIGHT*2/3, ori, landAngle, canvas, paint_info);				//for vertical
//							canvas.drawLine(0, GlobalParams.SCREEN_HEIGHT*2/3, GlobalParams.SCREEN_WIDTH, GlobalParams.SCREEN_HEIGHT*2/3, paint_target);	//for vertical
//							canvas.drawLine(0, SmartData[5]*ratioW, GlobalParams.SCREEN_WIDTH, SmartData[5]*ratioW, paint_target);							//for vertical					
							angleTracker(GlobalParams.SCREEN_WIDTH*2/3, GlobalParams.SCREEN_HEIGHT /2, canvas, paint_white, paint_target);
							guideDrawTextForHorizontal(GlobalParams.SCREEN_WIDTH*2/3, GlobalParams.SCREEN_HEIGHT /2, ori, landAngle, canvas, paint_info);
							canvas.drawLine( GlobalParams.SCREEN_WIDTH*2/3 , 0 , GlobalParams.SCREEN_WIDTH*2/3 , GlobalParams.SCREEN_HEIGHT , paint_target);						
							canvas.drawLine(SmartData[5]*ratioH ,0 ,SmartData[5]*ratioH, GlobalParams.SCREEN_HEIGHT, paint_target);
							
							break;
						case 90:
//							angleTracker(GlobalParams.SCREEN_WIDTH*2/3, GlobalParams.SCREEN_HEIGHT*1/2, canvas, paint_white, paint_target);
//							guideDrawText(GlobalParams.SCREEN_WIDTH*2/3, GlobalParams.SCREEN_HEIGHT*1/2, ori, landAngle, canvas, paint_info);
//							canvas.drawLine(GlobalParams.SCREEN_WIDTH*2/3, 0, GlobalParams.SCREEN_WIDTH*2/3, GlobalParams.SCREEN_HEIGHT, paint_target);
//							canvas.drawLine(SmartData[4]*ratioH, 0, SmartData[4]*ratioH, GlobalParams.SCREEN_HEIGHT, paint_target);
							
							angleTracker(GlobalParams.SCREEN_WIDTH*1/2, GlobalParams.SCREEN_HEIGHT/3, canvas, paint_white, paint_target);
							guideDrawTextForHorizontal(GlobalParams.SCREEN_WIDTH*1/2, GlobalParams.SCREEN_HEIGHT/3, ori, landAngle, canvas, paint_info);
							canvas.drawLine(0,GlobalParams.SCREEN_HEIGHT*1/3 ,GlobalParams.SCREEN_WIDTH, GlobalParams.SCREEN_HEIGHT*1/3, paint_target);
							canvas.drawLine(0 ,GlobalParams.SCREEN_HEIGHT-SmartData[4]*ratioH ,GlobalParams.SCREEN_WIDTH , GlobalParams.SCREEN_HEIGHT-SmartData[4]*ratioH, paint_target);
							break;
						case 180:
//							angleTracker(GlobalParams.SCREEN_WIDTH*1/2, GlobalParams.SCREEN_HEIGHT*1/3, canvas, paint_white, paint_target);
//							guideDrawText(GlobalParams.SCREEN_WIDTH*1/2, GlobalParams.SCREEN_HEIGHT*1/3, ori, landAngle, canvas, paint_info);
//							canvas.drawLine(0, GlobalParams.SCREEN_HEIGHT*1/3, GlobalParams.SCREEN_WIDTH, GlobalParams.SCREEN_HEIGHT*1/3, paint_target);
//							canvas.drawLine(0, SmartData[5]*ratioW, GlobalParams.SCREEN_WIDTH, SmartData[5]*ratioW, paint_target);							
							
							break;
						case 270:
//							angleTracker(GlobalParams.SCREEN_WIDTH*1/3, GlobalParams.SCREEN_HEIGHT*1/2, canvas, paint_white, paint_target);				// for vertical
//							guideDrawText(GlobalParams.SCREEN_WIDTH*1/3, GlobalParams.SCREEN_HEIGHT*1/2, ori, landAngle, canvas, paint_info);			// for vertical
//							canvas.drawLine(GlobalParams.SCREEN_WIDTH*1/3, 0, GlobalParams.SCREEN_WIDTH*1/3, GlobalParams.SCREEN_HEIGHT, paint_target);	// for vertical
//							canvas.drawLine(SmartData[4]*ratioH, 0, SmartData[4]*ratioH, GlobalParams.SCREEN_HEIGHT, paint_target);						// for vertical	
							angleTracker(GlobalParams.SCREEN_WIDTH*1/2, GlobalParams.SCREEN_HEIGHT*2/3, canvas, paint_white, paint_target);
							guideDrawTextForHorizontal(GlobalParams.SCREEN_WIDTH*1/2, GlobalParams.SCREEN_HEIGHT*2/3, ori, landAngle, canvas, paint_info);
							canvas.drawLine(0,GlobalParams.SCREEN_HEIGHT*2/3 ,GlobalParams.SCREEN_WIDTH, GlobalParams.SCREEN_HEIGHT*2/3, paint_target);
							canvas.drawLine(0 ,GlobalParams.SCREEN_HEIGHT-SmartData[4]*ratioH ,GlobalParams.SCREEN_WIDTH , GlobalParams.SCREEN_HEIGHT-SmartData[4]*ratioH, paint_target);
							
							break;
						default:
							break;				
						}
						
//						guideDrawLine((int)(SmartData[4]*ratioH),(int)(SmartData[5]*ratioW), (int)(SmartData[12]*ratioH),(int)(SmartData[13]*ratioW), canvas, paint_target);  // for vertical
						canvas.drawLine((int)(SmartData[5]*ratioW), GlobalParams.SCREEN_HEIGHT- (int)(SmartData[4]*ratioH), (int)(SmartData[13]*ratioW),GlobalParams.SCREEN_HEIGHT- (int)(SmartData[12]*ratioH), paint_target);//  for horizontal
					
						
//						int xj = (int)(SmartData[12]*ratioH)+((int)(SmartData[4]*ratioH)-(int)(SmartData[12]*ratioH))/2;     	// for vertical
//						int yj = (int)(SmartData[13]*ratioW)+((int)(SmartData[5]*ratioW) -(int)(SmartData[13]*ratioW))/2;		// for vertical
//						paint_target.setStrokeWidth(8);																			// for vertical
//						guideDrawAL((int)(SmartData[12]*ratioH),(int)(SmartData[13]*ratioW), xj,yj , canvas , paint_target);	// for vertical											
						int x_c =  SmartData[5];								// for horizontal
						int y_c =  GlobalParams.SCREEN_HEIGHT - SmartData[4];	// for horizontal
						int x_t =  SmartData[13];								// for horizontal
						int y_t =  GlobalParams.SCREEN_HEIGHT - SmartData[12];	// for horizontal
						paint_target.setStrokeWidth(8);							// for horizontal
						guideDrawAL(x_t,y_t, x_c,y_c , canvas , paint_target);	// for horizontal

					}else{
						
					}
				}
			} else {
			}
		}else{
//			//---------------portrait recommendation------------------------------------
//			if(SmartData[0] == FLAG_DETECTION_PORTRAIT && SmartData[1] == FLAG_GUIDE_ALLOW){
//				
//				paint_target.setStyle(Style.STROKE);
//				paint_target.setColor(Color.argb(80, 20, 150, 250));  //浅蓝
//				paint_target.setStrokeWidth(5); 		
//
//				paint_white.setStyle(Style.STROKE);
//				paint_white.setColor(Color.argb(60, 255, 255, 255));  //白色
//				paint_white.setStrokeWidth(15); 
//
//				paint_white_target.setStyle(Style.FILL_AND_STROKE);
//				paint_white_target.setColor(Color.argb(35, 255, 255, 255));  
//				paint_white_target.setStrokeWidth(15); 
//
//				paint_info.setStyle(Style.FILL);
//				paint_info.setTextSize(40);
//				paint_info.setTypeface(Typeface.DEFAULT_BOLD);
//				paint_info.setColor(Color.argb(255, 255, 255, 255)); 
//
//				String portraitRatio = SmartData[2] + "%";
//				int ratio = 0;
//				
//				if(SmartData[3] == 0){
//					//放大
//					if(ori == 0 || ori == 180){
//						ratio = (int)(2.5 * SmartData[2]);
//						paint_target.setStrokeWidth(5);
//						guideDrawCircle((int)(SmartData[4] * ratioH),
//										(int)(SmartData[5] * ratioW), 250, canvas, paint_target);
//						guideDrawCircle((int)(SmartData[4] * ratioH),
//										(int)(SmartData[5] * ratioW), 280, canvas, paint_target);			
//					}else{
//						ratio = (int)(1.5 * SmartData[2]);
//						paint_target.setStrokeWidth(5);
//						guideDrawCircle((int)(SmartData[4] * ratioH),
//										(int)(SmartData[5] * ratioW), 150, canvas, paint_target);
//						guideDrawCircle((int)(SmartData[4] * ratioH),
//										(int)(SmartData[5] * ratioW), 165, canvas, paint_target);			
//					}
//					guideDrawCircle((int)(SmartData[4] * ratioH),
//									(int)(SmartData[5] * ratioW), ratio, canvas, paint_white_target);
//					guideDrawText((int)(SmartData[4] * ratioH),
//								  (int)(SmartData[5] * ratioW), ori, portraitRatio, canvas, paint_info);
//					guideDrawTag((int)(SmartData[4] * ratioH),
//					             (int)(SmartData[5] * ratioW), 40, ori,  "Closer", canvas, paint_info);
//				}else if(SmartData[3] == 2){
//					//缩小
//					paint_target.setColor(Color.argb(80, 250, 80, 130)); //pink color
//					if(ori == 0 || ori == 180){
//						paint_white.setStrokeWidth(5);
//						guideDrawCircle((int)(SmartData[4] * ratioH),
//										(int)(SmartData[5] * ratioW), 250, canvas, paint_target);
//						guideDrawCircle((int)(SmartData[4] * ratioH),
//										(int)(SmartData[5] * ratioW), 280, canvas, paint_target);			
//					}else{
//						paint_white.setStrokeWidth(5);
//						guideDrawCircle((int)(SmartData[4] * ratioH),
//										(int)(SmartData[5] * ratioW), 150, canvas, paint_target);
//						guideDrawCircle((int)(SmartData[4] * ratioH),
//										(int)(SmartData[5] * ratioW), 165, canvas, paint_target);			
//					}
//					guideDrawText((int)(SmartData[4] * ratioH),
//							      (int)(SmartData[5] * ratioW), ori, "100%", canvas, paint_info);
//					guideDrawTag((int)(SmartData[4] * ratioH),
//				                 (int)(SmartData[5] * ratioW), 40, ori,  "Further", canvas, paint_info);
//				}else{
//					//合适大小
//					if(SmartData[2] >= 100){
//						portraitRatio = "100%";
//					}
//					
//					if(ori == 90 || ori == 270){
//						//横屏正脸
//						if(SmartData[7] == 2){
//							//距离 perfect
//							paint_white.setColor(Color.argb(80, 20, 150, 250));  //blue
//							paint_white.setStrokeWidth(15);
//							guideDrawCircle((int)(faceTarget[0] * ratioH), (int)(faceTarget[1] * ratioW),  150, canvas, paint_white, SmartData[8]);
//							guideDrawCircle((int)(faceTarget[2] * ratioH), (int)(faceTarget[3] * ratioW), 150, canvas, paint_white, SmartData[9]);	
//							guideDrawText((int)(SmartData[4] * ratioH),
//								  	  	  (int)(SmartData[5] * ratioW), ori, portraitRatio, canvas, paint_info);
//							guideDrawTag((int)(SmartData[4] * ratioH),
//							         	 (int)(SmartData[5] * ratioW), 80, ori,  "Prefect", canvas, paint_info);
//						}else if(SmartData[7] == 1){
//							//距离 good
//							paint_white.setColor(Color.argb(120, 255, 255, 255));  //白色
//							paint_white.setStrokeWidth(15);
//							guideDrawCircle((int)(faceTarget[0] * ratioH), (int)(faceTarget[1] * ratioW),  150, canvas, paint_white, SmartData[8]);
//							guideDrawCircle((int)(faceTarget[2] * ratioH), (int)(faceTarget[3] * ratioW), 150, canvas, paint_white, SmartData[9]);
//							paint_target.setColor(Color.argb(80, 20, 150, 250));  //blue color
//							guideDrawCircle((int)(SmartData[4] * ratioH),
//											(int)(SmartData[5] * ratioW), 150, canvas, paint_target);
//							guideDrawCircle((int)(SmartData[4] * ratioH),
//											(int)(SmartData[5] * ratioW), 165, canvas, paint_target);
//							guideDrawText((int)(SmartData[4] * ratioH),
//									  	  (int)(SmartData[5] * ratioW), ori, portraitRatio, canvas, paint_info);
//							guideDrawTag((int)(SmartData[4] * ratioH),
//						         	 	 (int)(SmartData[5] * ratioW), 80, ori,  "Good", canvas, paint_info);
//						}else{
//							//距离不达标
//							paint_white.setColor(Color.argb(120, 255, 255, 255));  //白色
//							paint_white.setStrokeWidth(15);
//							guideDrawCircle((int)(faceTarget[0] * ratioH), (int)(faceTarget[1] * ratioW),  150, canvas, paint_white, SmartData[8]);
//							guideDrawCircle((int)(faceTarget[2] * ratioH), (int)(faceTarget[3] * ratioW), 150, canvas, paint_white, SmartData[9]);
//							paint_target.setColor(Color.argb(80, 20, 150, 250));  //blue color
//							guideDrawCircle((int)(SmartData[4] * ratioH),
//											(int)(SmartData[5] * ratioW), 150, canvas, paint_target);
//							guideDrawCircle((int)(SmartData[4] * ratioH),
//											(int)(SmartData[5] * ratioW), 165, canvas, paint_target);
//							guideDrawText((int)(SmartData[4] * ratioH),
//									  	  (int)(SmartData[5] * ratioW), ori, portraitRatio, canvas, paint_info);
//						}	
//					}else{
//						//竖屏正脸
//						if(SmartData[7] == 2){
//							//距离perfect
//							paint_white.setColor(Color.argb(80, 20, 150, 250));  //blue
//							paint_white.setStrokeWidth(30);
//							guideDrawCircle((int)(faceTarget[4] * ratioH), (int)(faceTarget[5] * ratioW),  265, canvas, paint_white, 100);	
//							guideDrawText((int)(SmartData[4] * ratioH),
//								  	  	  (int)(SmartData[5] * ratioW), ori, portraitRatio, canvas, paint_info);
//							guideDrawTag((int)(SmartData[4] * ratioH),
//										 (int)(SmartData[5] * ratioW), 80, ori,  "Prefect", canvas, paint_info);
//						}else if(SmartData[7] == 1){
//							//距离 good
//							paint_white.setColor(Color.argb(120, 255, 255, 255));  //白色
//							paint_white.setStrokeWidth(30);
//							guideDrawCircle((int)(faceTarget[4] * ratioH), (int)(faceTarget[5] * ratioW),  265, canvas, paint_white, 100);
//							paint_target.setColor(Color.argb(80, 20, 150, 250));  //blue color
//							guideDrawCircle((int)(SmartData[4] * ratioH),
//											(int)(SmartData[5] * ratioW), 250, canvas, paint_target);
//							guideDrawCircle((int)(SmartData[4] * ratioH),
//											(int)(SmartData[5] * ratioW), 280, canvas, paint_target);
//							guideDrawText((int)(SmartData[4] * ratioH),
//									  	  (int)(SmartData[5] * ratioW), ori, portraitRatio, canvas, paint_info);
//							guideDrawTag((int)(SmartData[4] * ratioH),
//						         	 	 (int)(SmartData[5] * ratioW), 80, ori,  "Good", canvas, paint_info);
//						}else{
//							//距离不达标
//							paint_white.setColor(Color.argb(60, 255, 255, 255));  //白色
//							paint_white.setStrokeWidth(30);
//							guideDrawCircle((int)(faceTarget[4] * ratioH), (int)(faceTarget[5] * ratioW),  265, canvas, paint_white, 100);
//							paint_target.setColor(Color.argb(80, 20, 150, 250));  //blue color
//							guideDrawCircle((int)(SmartData[4] * ratioH),
//											(int)(SmartData[5] * ratioW), 250, canvas, paint_target);
//							guideDrawCircle((int)(SmartData[4] * ratioH),
//											(int)(SmartData[5] * ratioW), 280, canvas, paint_target);
//							guideDrawText((int)(SmartData[4] * ratioH),
//									  	  (int)(SmartData[5] * ratioW), ori, portraitRatio, canvas, paint_info);
//						}	
//					}
//				}	
//			}else{
//			}
//			
//			//---------------ellipse recommendation------------------------------------				
//			if (SmartData[0] == FLAG_DETECTION_FOOD && SmartData[1] == FLAG_GUIDE_ALLOW){
//				paint_target.setStyle(Style.STROKE);
//				paint_target.setColor(Color.argb(80, 20, 150, 250));  //浅蓝
//				paint_target.setStrokeWidth(5); 		
//
//				paint_white.setStyle(Style.STROKE);
//				paint_white.setColor(Color.argb(60, 255, 255, 255));  //白色
//				paint_white.setStrokeWidth(15); 
//
//				paint_white_target.setStyle(Style.FILL_AND_STROKE);
//				paint_white_target.setColor(Color.argb(35, 255, 255, 255));  
//				paint_white_target.setStrokeWidth(15); 
//
//				paint_info.setStyle(Style.FILL);
//				paint_info.setTextSize(40);
//				paint_info.setTypeface(Typeface.DEFAULT_BOLD);
//				paint_info.setColor(Color.argb(255, 255, 255, 255)); 
//				
//				int x0 = SmartSingleActivity.SCREEN_HEIGHT/2 ;
//				int y0 = GlobalParams.SCREEN_WIDTH; /2 ;
//				String foodRatio = SmartData[2] + "%";
//				if (SmartData[7] != 0) {
//					// ready Grab				
//					if(SmartData[7] == 2){
//						// perfect 判定 
////						guideDrawCircle((int)(SmartData[4] * ratioH),
////										(int)(SmartData[5] * ratioW), 150, canvas, paint_target);
////						guideDrawCircle((int)(SmartData[12]  * ratioH),
////										(int)(SmartData[13] * ratioW), 150, canvas, paint_white);
//						paint_white.setColor(Color.argb(80, 20, 150, 250));
//						paint_white.setStrokeWidth(15);
//						guideDrawCircle((int)(SmartData[12] * ratioH),
//								        (int)(SmartData[13] * ratioW), 150, canvas, paint_white);
//						guideDrawText((int)(SmartData[4] * ratioH),
//							          (int)(SmartData[5] * ratioW), ori, foodRatio, canvas, paint_info);
//						guideDrawTag((int)(SmartData[4] * ratioH),
//							         (int)(SmartData[5] * ratioW), 80, ori,  "Prefect", canvas, paint_info);
////						guideDrawText((int)(SmartData[4] * ratioH),
////								      (int)(SmartData[5] * ratioW + 80), ori,  "Prefect", canvas, paint_info);
//					}else{
//						// good 判定
//						paint_target.setColor(Color.argb(80, 20, 150, 250));  //blue color
//						guideDrawCircle((int)(SmartData[4] * ratioH),
//										(int)(SmartData[5] * ratioW), 150, canvas, paint_target);
//						guideDrawCircle((int)(SmartData[4] * ratioH),
//										(int)(SmartData[5] * ratioW), 165, canvas, paint_target);
//						
//						guideDrawCircle((int)(SmartData[12] * ratioH),
//										(int)(SmartData[13] * ratioW), 150, canvas, paint_white);
//						guideDrawText((int)(SmartData[4] * ratioH),
//								      (int)(SmartData[5] * ratioW), ori, foodRatio, canvas, paint_info);
//						guideDrawTag((int)(SmartData[4] * ratioH),
//						             (int)(SmartData[5] * ratioW), 80, ori,  "Good", canvas, paint_info);
////						guideDrawText((int)(SmartData[4] * ratioH),
////								      (int)(SmartData[5] * ratioW + 80), ori, "Good", canvas, paint_info);
//					}
//				} else {
//					// not ready Grab
//
//					if(SmartData[3] == 0){
//						//放大
//						int ratio = (int)(1.5 * SmartData[2]);
//						guideDrawCircle((int)(SmartData[4] * ratioH),
//										(int)(SmartData[5] * ratioW), 150, canvas, paint_white);
//						guideDrawCircle((int)(SmartData[4] * ratioH),
//										(int)(SmartData[5] * ratioW), ratio, canvas, paint_white_target);
//						guideDrawText((int)(SmartData[4] * ratioH),
//									  (int)(SmartData[5] * ratioW), ori, foodRatio, canvas, paint_info);
//						guideDrawTag((int)(SmartData[4] * ratioH),
//						             (int)(SmartData[5] * ratioW), 40, ori,  "Closer", canvas, paint_info);
//					}else if(SmartData[3] == 2){
//						//缩小
//						paint_target.setColor(Color.argb(80, 250, 80, 130)); //pink color
//						guideDrawCircle((int)(SmartData[4] * ratioH),
//										(int)(SmartData[5] * ratioW), 150, canvas, paint_target);
//						guideDrawCircle((int)(SmartData[4] * ratioH),
//										(int)(SmartData[5] * ratioW), 165, canvas, paint_target);
//						guideDrawText((int)(SmartData[4] * ratioH),
//								      (int)(SmartData[5] * ratioW), ori, "100%", canvas, paint_info);
//						guideDrawTag((int)(SmartData[4] * ratioH),
//					                 (int)(SmartData[5] * ratioW), 40, ori,  "Further", canvas, paint_info);
//					}else{
//						//合适大小
//						paint_target.setColor(Color.argb(80, 20, 150, 250));  //blue color
//						guideDrawCircle((int)(SmartData[4] * ratioH),
//										(int)(SmartData[5] * ratioW), 150, canvas, paint_target);
//						guideDrawCircle((int)(SmartData[4] * ratioH),
//										(int)(SmartData[5] * ratioW), 165, canvas, paint_target);
//						guideDrawCircle((int)(SmartData[12] * ratioH),
//										(int)(SmartData[13] * ratioW), 150, canvas, paint_white);
//						guideDrawText((int)(SmartData[4] * ratioH),
//								  	  (int)(SmartData[5] * ratioW), ori, foodRatio, canvas, paint_info);
//					}
//				}
//			} else {	
//			}
//			
//			//---------------line recommendation------------------------------------
//			//-------------draw Line---------------------------------------------			
//			if (SmartData[0] == FLAG_DETECTION_LANDSCAPE && SmartData[1] == FLAG_GUIDE_ALLOW){
//				
//				paint_target.setStyle(Style.STROKE);
//				paint_target.setColor(Color.argb(80, 20, 150, 250));  //浅蓝
//				paint_target.setStrokeWidth(5); 	
//				
//				paint_white.setStyle(Style.STROKE);
//				paint_white.setColor(Color.argb(60, 255, 255, 255));  //白色
//				paint_white.setStrokeWidth(15);
//				
//				paint_info.setStyle(Style.FILL);
//				paint_info.setTextSize(40);
//				paint_info.setTypeface(Typeface.DEFAULT_BOLD);
//				paint_info.setColor(Color.argb(255, 255, 255, 255));
//				
//				String landAngle = SmartData[2] + "°";
//				
//				if (SmartData[7] != 0) {
//					// ready Grab
//					
//					if(SmartData[7] == 1){
//						// Good
//						paint_target.setColor(Color.argb(100, 20, 150, 250));  //浅蓝
//						paint_target.setStrokeWidth(4);					
//						paint_white.setColor(Color.argb(40, 255, 255, 255));  //白色	
//						switch(SmartData[6]){
//						case 0:
//							angleTracker(SmartSingleActivity.SCREEN_WIDTH*1/2, SmartSingleActivity.SCREEN_HEIGHT*2/3, canvas, paint_white, paint_target);
//							guideDrawText(SmartSingleActivity.SCREEN_WIDTH*1/2, SmartSingleActivity.SCREEN_HEIGHT*2/3, ori, landAngle, canvas, paint_info);
//							guideDrawTag(SmartSingleActivity.SCREEN_WIDTH*1/2, SmartSingleActivity.SCREEN_HEIGHT*2/3, 80, ori,  "Good", canvas, paint_info);					
//							canvas.drawLine(0, SmartSingleActivity.SCREEN_HEIGHT*2/3, SmartSingleActivity.SCREEN_WIDTH, SmartSingleActivity.SCREEN_HEIGHT*2/3, paint_target);
//							break;
//						case 90:		
//							angleTracker(SmartSingleActivity.SCREEN_WIDTH*2/3, SmartSingleActivity.SCREEN_HEIGHT*1/2, canvas, paint_white, paint_target);
//							guideDrawText(SmartSingleActivity.SCREEN_WIDTH*2/3, SmartSingleActivity.SCREEN_HEIGHT*1/2, ori, landAngle, canvas, paint_info);
//							guideDrawTag(SmartSingleActivity.SCREEN_WIDTH*2/3, SmartSingleActivity.SCREEN_HEIGHT*1/2, 80, ori,  "Good", canvas, paint_info);					
//							canvas.drawLine(SmartSingleActivity.SCREEN_WIDTH*2/3, 0, SmartSingleActivity.SCREEN_WIDTH*2/3, SmartSingleActivity.SCREEN_HEIGHT, paint_target);
//							break;
//						case 180:
//							angleTracker(SmartSingleActivity.SCREEN_WIDTH*1/2, SmartSingleActivity.SCREEN_HEIGHT*1/3, canvas, paint_white, paint_target);
//							guideDrawText(SmartSingleActivity.SCREEN_WIDTH*1/2, SmartSingleActivity.SCREEN_HEIGHT*1/3, ori, landAngle, canvas, paint_info);
//							guideDrawTag(SmartSingleActivity.SCREEN_WIDTH*1/2, SmartSingleActivity.SCREEN_HEIGHT*1/3, 80, ori,  "Good", canvas, paint_info);
//							canvas.drawLine(0, SmartSingleActivity.SCREEN_HEIGHT*1/3, SmartSingleActivity.SCREEN_WIDTH, SmartSingleActivity.SCREEN_HEIGHT*1/3, paint_target);
//							break;
//						case 270:
//							angleTracker(SmartSingleActivity.SCREEN_WIDTH*1/3, SmartSingleActivity.SCREEN_HEIGHT*1/2, canvas, paint_white, paint_target);
//							guideDrawText(SmartSingleActivity.SCREEN_WIDTH*1/3, SmartSingleActivity.SCREEN_HEIGHT*1/2, ori, landAngle, canvas, paint_info);
//							guideDrawTag(SmartSingleActivity.SCREEN_WIDTH*1/3, SmartSingleActivity.SCREEN_HEIGHT*1/2, 80, ori,  "Good", canvas, paint_info);
//							canvas.drawLine(SmartSingleActivity.SCREEN_WIDTH*1/3, 0, SmartSingleActivity.SCREEN_WIDTH*1/3, SmartSingleActivity.SCREEN_HEIGHT, paint_target);
//							break;
//						default:
//							break;				
//						}
//					}else{
//						// Perfect
//						paint_target.setColor(Color.argb(100, 20, 150, 250));  //浅蓝   //20, 150, 250
//						paint_target.setStrokeWidth(4);					
//						paint_white.setColor(Color.argb(40, 20, 150, 250));  //浅蓝	
//						switch(SmartData[6]){
//						case 0:
//							angleTracker(SmartSingleActivity.SCREEN_WIDTH*1/2, SmartSingleActivity.SCREEN_HEIGHT*2/3, canvas, paint_white, paint_target);
//							guideDrawText(SmartSingleActivity.SCREEN_WIDTH*1/2, SmartSingleActivity.SCREEN_HEIGHT*2/3, ori, landAngle, canvas, paint_info);
//							guideDrawTag(SmartSingleActivity.SCREEN_WIDTH*1/2, SmartSingleActivity.SCREEN_HEIGHT*2/3, 80, ori,  "Perfect", canvas, paint_info);					
//							canvas.drawLine(0, SmartSingleActivity.SCREEN_HEIGHT*2/3, SmartSingleActivity.SCREEN_WIDTH, SmartSingleActivity.SCREEN_HEIGHT*2/3, paint_target);
//							break;
//						case 90:		
//							angleTracker(SmartSingleActivity.SCREEN_WIDTH*2/3, SmartSingleActivity.SCREEN_HEIGHT*1/2, canvas, paint_white, paint_target);
//							guideDrawText(SmartSingleActivity.SCREEN_WIDTH*2/3, SmartSingleActivity.SCREEN_HEIGHT*1/2, ori, landAngle, canvas, paint_info);
//							guideDrawTag(SmartSingleActivity.SCREEN_WIDTH*2/3, SmartSingleActivity.SCREEN_HEIGHT*1/2, 80, ori,  "Perfect", canvas, paint_info);					
//							canvas.drawLine(SmartSingleActivity.SCREEN_WIDTH*2/3, 0, SmartSingleActivity.SCREEN_WIDTH*2/3, SmartSingleActivity.SCREEN_HEIGHT, paint_target);
//							break;
//						case 180:
//							angleTracker(SmartSingleActivity.SCREEN_WIDTH*1/2, SmartSingleActivity.SCREEN_HEIGHT*1/3, canvas, paint_white, paint_target);
//							guideDrawText(SmartSingleActivity.SCREEN_WIDTH*1/2, SmartSingleActivity.SCREEN_HEIGHT*1/3, ori, landAngle, canvas, paint_info);
//							guideDrawTag(SmartSingleActivity.SCREEN_WIDTH*1/2, SmartSingleActivity.SCREEN_HEIGHT*1/3, 80, ori,  "Perfect", canvas, paint_info);
//							canvas.drawLine(0, SmartSingleActivity.SCREEN_HEIGHT*1/3, SmartSingleActivity.SCREEN_WIDTH, SmartSingleActivity.SCREEN_HEIGHT*1/3, paint_target);
//							break;
//						case 270:
//							angleTracker(SmartSingleActivity.SCREEN_WIDTH*1/3, SmartSingleActivity.SCREEN_HEIGHT*1/2, canvas, paint_white, paint_target);
//							guideDrawText(SmartSingleActivity.SCREEN_WIDTH*1/3, SmartSingleActivity.SCREEN_HEIGHT*1/2, ori, landAngle, canvas, paint_info);
//							guideDrawTag(SmartSingleActivity.SCREEN_WIDTH*1/3, SmartSingleActivity.SCREEN_HEIGHT*1/2, 80, ori,  "Perfect", canvas, paint_info);
//							canvas.drawLine(SmartSingleActivity.SCREEN_WIDTH*1/3, 0, SmartSingleActivity.SCREEN_WIDTH*1/3, SmartSingleActivity.SCREEN_HEIGHT, paint_target);
//							break;
//						default:
//							break;				
//						}
//					}
//				} else {
//					// not ready Grab			
//					paint_target.setColor(Color.argb(100, 255, 255, 255));  //白色
//					paint_target.setStrokeWidth(4);					
//					paint_white.setColor(Color.argb(40, 255, 255, 255));    //白色
//					if(Math.abs(SmartData[2]) <= 6){
//						paint_recomm.setColor(Color.argb(80, 255, 0, 0)); // red color
//						switch(SmartData[6]){
//						case 0:
//							angleTracker(SmartSingleActivity.SCREEN_WIDTH*1/2, SmartSingleActivity.SCREEN_HEIGHT*2/3, canvas, paint_white, paint_target);
//							guideDrawText(SmartSingleActivity.SCREEN_WIDTH*1/2, SmartSingleActivity.SCREEN_HEIGHT*2/3, ori, landAngle, canvas, paint_info);
//							canvas.drawLine(0, SmartSingleActivity.SCREEN_HEIGHT*2/3, SmartSingleActivity.SCREEN_WIDTH, SmartSingleActivity.SCREEN_HEIGHT*2/3, paint_target);
//							canvas.drawLine(0, SmartData[5]*ratioW, SmartSingleActivity.SCREEN_WIDTH, SmartData[5]*ratioW, paint_target);
//							break;
//						case 90:
//							angleTracker(SmartSingleActivity.SCREEN_WIDTH*2/3, SmartSingleActivity.SCREEN_HEIGHT*1/2, canvas, paint_white, paint_target);
//							guideDrawText(SmartSingleActivity.SCREEN_WIDTH*2/3, SmartSingleActivity.SCREEN_HEIGHT*1/2, ori, landAngle, canvas, paint_info);
//							canvas.drawLine(SmartSingleActivity.SCREEN_WIDTH*2/3, 0, SmartSingleActivity.SCREEN_WIDTH*2/3, SmartSingleActivity.SCREEN_HEIGHT, paint_target);
//							canvas.drawLine(SmartData[4]*ratioH, 0, SmartData[4]*ratioH, SmartSingleActivity.SCREEN_HEIGHT, paint_target);
//							break;
//						case 180:
//							angleTracker(SmartSingleActivity.SCREEN_WIDTH*1/2, SmartSingleActivity.SCREEN_HEIGHT*1/3, canvas, paint_white, paint_target);
//							guideDrawText(SmartSingleActivity.SCREEN_WIDTH*1/2, SmartSingleActivity.SCREEN_HEIGHT*1/3, ori, landAngle, canvas, paint_info);
//							canvas.drawLine(0, SmartSingleActivity.SCREEN_HEIGHT*1/3, SmartSingleActivity.SCREEN_WIDTH, SmartSingleActivity.SCREEN_HEIGHT*1/3, paint_target);
//							canvas.drawLine(0, SmartData[5]*ratioW, SmartSingleActivity.SCREEN_WIDTH, SmartData[5]*ratioW, paint_target);
//							break;
//						case 270:
//							angleTracker(SmartSingleActivity.SCREEN_WIDTH*1/3, SmartSingleActivity.SCREEN_HEIGHT*1/2, canvas, paint_white, paint_target);
//							guideDrawText(SmartSingleActivity.SCREEN_WIDTH*1/3, SmartSingleActivity.SCREEN_HEIGHT*1/2, ori, landAngle, canvas, paint_info);
//							canvas.drawLine(SmartSingleActivity.SCREEN_WIDTH*1/3, 0, SmartSingleActivity.SCREEN_WIDTH*1/3, SmartSingleActivity.SCREEN_HEIGHT, paint_target);
//							canvas.drawLine(SmartData[4]*ratioH, 0, SmartData[4]*ratioH, SmartSingleActivity.SCREEN_HEIGHT, paint_target);
//							break;
//						default:
//							break;				
//						}
//						
//						guideDrawLine((int)(SmartData[4]*ratioH),
//									  (int)(SmartData[5]*ratioW),
//								      (int)(SmartData[12]*ratioH),
//								      (int)(SmartData[13]*ratioW), canvas, paint_target);
//						int xj = (int)(SmartData[12]*ratioH)
//								+((int)(SmartData[4]*ratioH)
//								-(int)(SmartData[12]*ratioH))/2;
//						int yj = (int)(SmartData[13]*ratioW)
//								+((int)(SmartData[5]*ratioW)
//							    -(int)(SmartData[13]*ratioW))/2;
//						paint_target.setStrokeWidth(8);	
//						guideDrawAL((int)(SmartData[12]*ratioH),
//								    (int)(SmartData[13]*ratioW), xj,yj , canvas , paint_target);
//					}else{
//						
//					}
//				}
//			} else {
//			}	
		}
			
		invalidate();

		super.onDraw(canvas);
		
		float ratioW_Result = 1.0f;
		float ratioH_Result = 1.0f;
		int ori_Result = 0;
		int x = 210;
		int y = 50;
		String scene_info_str = " ";
		String scene_motion_info_str = " ";
		
		switch(mSceneFlag)
		{
		case FLAG_DETECTION_ERROR:
			scene_info_str = "SceneFlag: ERROR";
			break;
		case FLAG_DETECTION_NORMAL:
			scene_info_str = "SceneFlag: NORMAL";
			break;
		case FLAG_DETECTION_PORTRAIT:
			scene_info_str = "SceneFlag: PORTRAIT";
			break;
		case FLAG_DETECTION_LOWLIT:
			scene_info_str = "SceneFlag: LOWLIT";
			break;
		case FLAG_DETECTION_ULTRA_LOWLIT:
			scene_info_str = "SceneFlag: ULTRA LOWLIT";
			break;
		case FLAG_DETECTION_PORTRAIT_BACKLIT:
			scene_info_str = "SceneFlag: PORTRAIT BACKLIT";
			break;
		case FLAG_DETECTION_PORTRAIT_LOWLIT:
			scene_info_str = "SceneFlag: PORTRAIT LOWLIT";
			break;
		case FLAG_DETECTION_BACKLIGHT:
			scene_info_str = "SceneFlag: BACKLIGHT";
			break;
		case FLAG_DETECTION_LANDSCAPE:
			scene_info_str = "SceneFlag: LANDSCAPE";
			break;
		case FLAG_DETECTION_FOOD:
			scene_info_str = "SceneFlag: FOOD";
			break;
		case FLAG_DETECTION_QRCODE:
			scene_info_str = "SceneFlag: QRCODE";
			break;
		case FLAG_DETECTION_DOCUMENT:
			scene_info_str = "SceneFlag: DOCUMENT";
			break;
		case FLAG_DETECTION_JITTERING:
			scene_info_str = "SceneFlag: JITTERING";
			break;
		case FLAG_DETECTION_MOVE:
			scene_info_str = "SceneFlag: MOVE";
			break;
		default:
			scene_info_str = "SceneFlag: ";
			break;
				
		}
		
		switch(mSceneMotionFlag)
		{
		case FLAG_DETECTION_JITTERING:
			scene_motion_info_str = "\nJITTERING";
			break;
		case FLAG_DETECTION_MOVE:
			scene_motion_info_str = "\nMOVE";
			break;
		default:
			scene_motion_info_str = " ";
			break;
				
		}
		
		guideDrawTagNoRotate((int)(x * ratioH_Result), (int)(y * ratioW_Result), 40, ori_Result,  scene_info_str + scene_motion_info_str, canvas, paint_Result);
	}

	
	Paint paint_r = new Paint();
	Paint paint_g = new Paint();
    Paint paint_b = new Paint();
    Paint paint_y = new Paint();
    Paint paint_target = new Paint();
    Paint paint_recomm = new Paint();
    Paint paint_white = new Paint();
    Paint paint_white_target = new Paint();
    Paint paint_info = new Paint();
	Paint paint_zoom  = new Paint();

	int[] face = new int[21];
	int[] faceTarget = new int[6];
	
    public  void angleTracker(int x , int y , Canvas canvas , Paint mPaint1, Paint mPaint2){	
    	mPaint1.setStrokeWidth(80);
//    	guideDrawCircle(y, GlobalParams.SCREEN_HEIGHT - x, 110, canvas, mPaint1);
//		guideDrawCircle(y, GlobalParams.SCREEN_HEIGHT - x, 150, canvas, mPaint2);
    	canvas.drawCircle(x, y, 110, mPaint1);
    	canvas.drawCircle(x,y, 150, mPaint2);
		mPaint1.setStrokeWidth(20);
//		guideDrawCircle(y, GlobalParams.SCREEN_HEIGHT - x, 40, canvas, mPaint1);
//		guideDrawCircle(y, GlobalParams.SCREEN_HEIGHT - x, 50, canvas, mPaint2);
		canvas.drawCircle(x, y, 40, mPaint1);
		canvas.drawCircle(x, y, 50, mPaint2);
    	
//    	mPaint1.setStrokeWidth(80);
//    	guideDrawCircle(x, y, 110, canvas, mPaint1);
//		guideDrawCircle(x, y, 150, canvas, mPaint2);
//		mPaint1.setStrokeWidth(20);
//		guideDrawCircle(x, y, 40, canvas, mPaint1);
//		guideDrawCircle(x, y, 50, canvas, mPaint2);
    }
	
    public  void guideDrawCircle(int x , int y , int r , Canvas canvas , Paint mPaint ){	
		canvas.drawCircle(y, GlobalParams.SCREEN_HEIGHT - x, r, mPaint);          // 后置人像位置
    }
    public  void guideDrawCircle(int x , int y , int r , Canvas canvas , Paint mPaint , int alphy ){	
    	mPaint.setAlpha(alphy);
		canvas.drawCircle(x, y, r, mPaint);          // 后置人像位置
    }

    
    public  void guideDrawLine(int x1 , int y1 , int x2 , int y2 , Canvas canvas , Paint mPaint){
    	    
//		canvas.drawLine(x1, y1, x2, y2, mPaint);// 后置人像位置
		canvas.drawLine(GlobalParams.SCREEN_WIDTH- y1, x1, GlobalParams.SCREEN_WIDTH- y2, x2, mPaint);//  for vertical
//		canvas.drawLine(x1, y1, x2, y2, mPaint);//  for horizontal
	  //canvas.drawCircle(Util.screenWidth- y,Util.screenHeight-x, r, paint_recomm);    // 前置人像位置
    }
    
    public void guideDrawAL(int sx, int sy, int ex, int ey , Canvas canvas , Paint mPaint)  
    {  
//    	int x1 = sx , y1 = sy , x2 = ex , y2 = ey;  //
//    	
//    	sx = CameraActivity.SCREEN_WIDTH- y1;
//    	sy = x1;
//    	ex = CameraActivity.SCREEN_WIDTH- y2;
//    	ey = x2;
    	
        double H = 8; // 箭头高度     
        double L = 3.5; // 底边的一半     
        int x3 = 0;  
        int y3 = 0;  
        int x4 = 0;  
        int y4 = 0;  
        double awrad = Math.atan(L / H); // 箭头角度     
        double arraow_len = Math.sqrt(L * L + H * H); // 箭头的长度     
        double[] arrXY_1 = rotateVec(ex - sx, ey - sy, awrad, true, arraow_len);  
        double[] arrXY_2 = rotateVec(ex - sx, ey - sy, -awrad, true, arraow_len);  
        double x_3 = ex - arrXY_1[0]; // (x3,y3)是第一端点     
        double y_3 = ey - arrXY_1[1];  
        double x_4 = ex - arrXY_2[0]; // (x4,y4)是第二端点     
        double y_4 = ey - arrXY_2[1];  
        Double X3 = new Double(x_3);  
        x3 = X3.intValue();  
        Double Y3 = new Double(y_3);  
        y3 = Y3.intValue();  
        Double X4 = new Double(x_4);  
        x4 = X4.intValue();  
        Double Y4 = new Double(y_4);  
        y4 = Y4.intValue();  
        // 画线     
        canvas.drawLine(sx, sy, ex, ey,mPaint);  
        Path triangle = new Path();  
        triangle.moveTo(ex, ey);  
        triangle.lineTo(x3, y3);    
        triangle.lineTo(x4, y4);   
        triangle.close();  
        canvas.drawPath(triangle,mPaint);  
  
    }
    public double[] rotateVec(int px, int py, double ang, boolean isChLen, double newLen)  
    {  
        double mathstr[] = new double[2];  
        // 矢量旋转函数，参数含义分别是x分量、y分量、旋转角、是否改变长度、新长度     
        double vx = px * Math.cos(ang) - py * Math.sin(ang);  
        double vy = px * Math.sin(ang) + py * Math.cos(ang);  
        if (isChLen) {  
            double d = Math.sqrt(vx * vx + vy * vy);  
            vx = vx / d * newLen;  
            vy = vy / d * newLen;  
            mathstr[0] = vx;  
            mathstr[1] = vy;  
        }  
        return mathstr;  
    }
    //修改20160224 设定屏幕方向为landscape后，屏幕坐标系相对原来顺时针旋转了90°，原来的Y相当于新坐标的X，原来的Height-X相当于新坐标的Y（Height是短边）
	  //display information
    public void guideDrawTag(int x , int y , int offset, float angle, String text,  Canvas canvas, Paint mPaint){
    	switch((int)angle){
    	case 0:
    		guideDrawTextNoRotate(y + offset, GlobalParams.SCREEN_HEIGHT - x, angle, text, canvas, mPaint);
    		 break;
    	case 90:
    		guideDrawTextNoRotate(y, GlobalParams.SCREEN_HEIGHT - x - offset, angle, text, canvas, mPaint);
    		break;
    	case 180:
    		guideDrawTextNoRotate(y - offset, GlobalParams.SCREEN_HEIGHT - x, angle, text, canvas, mPaint);
    		break;
    	case 270:
    		guideDrawTextNoRotate(y, GlobalParams.SCREEN_HEIGHT - x + offset, angle, text, canvas, mPaint);
    		break;
    	default:
    		break;
    		
//    	case 0:
//    		guideDrawTextNoRotate(x, y + offset,  angle, text, canvas, mPaint);
//    		 break;
//    	case 90:
//    		guideDrawTextNoRotate(x - offset ,y, angle, text, canvas, mPaint);
//    		break;
//    	case 180:
//    		guideDrawTextNoRotate(x , y - offset,  angle, text, canvas, mPaint);
//    		break;
//    	case 270:
//    		guideDrawTextNoRotate(x + offset , y, angle, text, canvas, mPaint);
//    		break;
//    	default:
//    		break;
    	}
    }
    
    public void guideDrawTagForHorizontal(int x , int y , int offset, float angle, String text,  Canvas canvas, Paint mPaint){
    	switch((int)angle){
    	case 0:
//    		 guideDrawTextNoRotate(y + offset, GlobalParams.SCREEN_HEIGHT - x, angle, text, canvas, mPaint);
    		 canvas.drawText(text, x+offset, y, mPaint);
    		 break;
    	case 90:
//    		guideDrawTextNoRotate(y, GlobalParams.SCREEN_HEIGHT - x - offset, angle, text, canvas, mPaint);
    		canvas.drawText(text, x, y-offset, mPaint);
    		break;
    	case 180:
//    		guideDrawTextNoRotate(y - offset, GlobalParams.SCREEN_HEIGHT - x, angle, text, canvas, mPaint);
    		canvas.drawText(text, x-offset, y, mPaint);
    		break;
    	case 270:
//    		guideDrawTextNoRotate(y, GlobalParams.SCREEN_HEIGHT - x + offset, angle, text, canvas, mPaint);
    		canvas.drawText(text, x, y+offset, mPaint);
    		break;
    	default:
    		break;
 
    	}
    }
    
    public void guideDrawText(int x , int y , float angle, String text,  Canvas canvas, Paint mPaint){
//        if(angle != 0){
//            canvas.rotate(360-angle, y, SmartSingleActivity.SCREEN_HEIGHT - x); 
//        }
    	canvas.drawText(text, y, GlobalParams.SCREEN_HEIGHT - x, mPaint);  // for vertical
 
//        if(angle != 0){
//            canvas.rotate(angle-360, y, SmartSingleActivity.SCREEN_HEIGHT - x); 
//        }
    }
    
    public void guideDrawTextForHorizontal(int x , int y , float angle, String text,  Canvas canvas, Paint mPaint){
//      if(angle != 0){
//          canvas.rotate(360-angle, y, SmartSingleActivity.SCREEN_HEIGHT - x); 
//      }

  	canvas.drawText(text, x, y, mPaint);  // for horizatal
//      if(angle != 0){
//          canvas.rotate(angle-360, y, SmartSingleActivity.SCREEN_HEIGHT - x); 
//      }
  }
    
    public void guideDrawTagNoRotate(int x , int y , int offset, float angle, String text,  Canvas canvas, Paint mPaint){
    	switch((int)angle){
    	case 0:
    		guideDrawTextNoRotate(x, y + offset, angle, text, canvas, mPaint);
    		 break;
    	case 90:
    		guideDrawTextNoRotate(x + offset, y, angle, text, canvas, mPaint);
    		break;
    	case 180:
    		guideDrawTextNoRotate(x, y - offset, angle, text, canvas, mPaint);
    		break;
    	case 270:
    		guideDrawTextNoRotate(x - offset, y, angle, text, canvas, mPaint);
    		break;
    	default:
    		break;
    	}
    }
    public void guideDrawTextNoRotate(int x , int y , float angle, String text,  Canvas canvas, Paint mPaint){
//        if(angle != 0){
//            canvas.rotate(360-angle, x, y); 
//        }
        canvas.drawText(text, x, y, mPaint);
//        if(angle != 0){
//            canvas.rotate(angle-360, x, y); 
//        }
    }
    
    
}
