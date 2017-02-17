/*
 *  PreviewRenderer.java
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

package com.lenovo.smartengine.view.renderer;

import static android.opengl.GLES20.glViewport;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

import android.content.Context;
import android.opengl.GLES20;
import android.opengl.GLSurfaceView;
import android.util.Log;

import com.example.arframetest.R;
import com.lenovo.smartengine.gl.model.GLSmartCube3D;
//import com.lenovo.smartengine.gl.model.GLSmartParticles;
//import com.lenovo.smartengine.gl.model.GLSmartPlugin;/
import com.lenovo.smartengine.gl.model.GLSmartRGB2D;
//import com.lenovo.smartengine.gl.model.GLSmartVideo;
import com.lenovo.smartengine.gl.model.GLSmartYUV2D;
import com.lenovo.smartengine.jni.SmartNative;
import com.lenovo.smartengine.util.GlobalParams;
import com.lenovo.smartengine.view.CameraSurfaceView;

public class SmartSingleRenderer implements GLSurfaceView.Renderer {

	private static final String TAG = "SmartSingleRenderer";

	private GLSmartYUV2D mPreviewRenderer;
	private GLSmartCube3D mCubeRenderer;
//	private GLSmartPlugin glPlugin;
//	private GLSmartVideo mSmartVideo;
	public static GLSmartRGB2D mRGBRenderer;
	private final Context context;
	
//	private GLSmartParticles glParticles = null ;

	public SmartSingleRenderer(Context context) {
		this.context = context;
//		glPlugin = new GLSmartPlugin(context);
//		glParticles = new GLSmartParticles(context);
	}

	public void onSurfaceCreated(GL10 gl, EGLConfig config) {

		mPreviewRenderer = GLSmartYUV2D.Create(context);
		mCubeRenderer = GLSmartCube3D.Create(context);
		mRGBRenderer = GLSmartRGB2D.Create(context);
//		mSmartVideo = GLSmartVideo.Create(context);
//		mSmartVideo.startVideo();
//		glPlugin.SmartGLRenderPligginPicking(true);
//		glPlugin.SmartGLRenderPlugginOnSurfaceCreated(R.raw.breadbonesright, R.raw.bread_texture);
//		glPlugin.SmartGLRenderPlugginOnSurfaceCreated(R.raw.fly_crunch_twomotion, R.raw.fly_body);
		
//		glParticles.GLSmartParticlesOnSurfaceCreated();
		
		Log.e(TAG, "加载模型成功");
	}

	public void onSurfaceChanged(GL10 unused, int width, int height) {
		glViewport(0, 0, width, height);
//		glPlugin.SmartGLRenderPlugginOnSurfaceChanged(width, height);
//		glParticles.GLSmartParticlesOnSurfaceChanged(width, height);
	}

	public void onDrawFrame(GL10 unused) {
		
		GLES20.glClear(GLES20.GL_COLOR_BUFFER_BIT | GLES20.GL_DEPTH_BUFFER_BIT);
//		GLES20.glClearColor(0f, 0.5f, 0.5f, 1f);
		//match the GL view port size with camera screen size 
		GLES20.glViewport(0, 0, GlobalParams.SCREEN_WIDTH, GlobalParams.SCREEN_HEIGHT);
//		GLES20.glViewport(0, 0, 1920, 1080);
		GLES20.glDisable(GLES20.GL_DEPTH_TEST);
		// mPreviewRenderer.draw();
		// facee
		if (GlobalParams.faceGLassFlag && CameraSurfaceView.faceFlag) {
			// rgb start
			mRGBRenderer.draw();
		} else {
			mPreviewRenderer.draw();
		}
		float[] porjectionMat = new float[16];
		porjectionMat = SmartNative.getMVPMatrix();

		Log.i("ARPorjectMatLog", "------->2" + "[" + porjectionMat[0] + "]"
				+ "[" + porjectionMat[1] + "]" + "[" + porjectionMat[2] + "]"
				+ "[" + porjectionMat[3] + "]" + "[" + porjectionMat[4] + "]"
				+ "[" + porjectionMat[5] + "]" + "[" + porjectionMat[6] + "]"
				+ "[" + porjectionMat[7] + "]" + "[" + porjectionMat[8] + "]"
				+ "[" + porjectionMat[9] + "]" + "[" + porjectionMat[10] + "]"
				+ "[" + porjectionMat[11] + "]" + "[" + porjectionMat[12] + "]"
				+ "[" + porjectionMat[13] + "]" + "[" + porjectionMat[14] + "]"
				+ "[" + porjectionMat[15] + "]");

		if (porjectionMat[0] != 0) {

			if (CameraSurfaceView.sceneResult[0] == 8) {
				// Food scene
				// ////////////////////////draw 3d model start
				float faceScore = SmartNative.getFoodAttractiveness();
				String foodString = "Attractiveness:" + " "
						+ String.valueOf(faceScore);
				Log.i("ARPorjectMatLog", "food " + foodString);

				float[] cubeMatrix = SmartNative.getMVPMatrix();
				if (foodModelFlag == FOOD_DISPLAY_FLY) {
					
//					float[] tempM = ARNativeActivity.getProjectM();
/*					if(cubeMatrix[0] > 0){
						glPlugin.SmartGLRenderPlugginOnDramFrame(SmartNative.getGlProjectionMatrix(),SmartNative.getExtrinsicMatrix());
					}else{
						glPlugin.isbegin = true;
					}*/
//					glPlugin.SmartGLRenderPlugginOnDramFrame(SmartNative.getGlProjectionMatrix(),SmartNative.getExtrinsicMatrix());
					
//					glPlugin.SmartGLRenderPlugginOnDramFrame(SmartNative.getGlProjectionMatrix(),SmartNative.getExtrinsicMatrix());
					
//					mSmartVideo.stopVideo();
				}else if(foodModelFlag == FOOD_DISPLAY_FOG){
//					glParticles.GLSmartParticlesOnDramFrame(cubeMatrix);
				}/*else if(foodModelFlag == FOOD_MODLE_FLAG_VIDEO)
				{
					if(!mSmartVideo.getPlayingFlag())
						mSmartVideo.startVideo();
					else					
					    mSmartVideo.draw(GlobalParams.SCREEN_WIDTH, GlobalParams.SCREEN_HEIGHT, porjectionMat);
				}*/
				 else
				{
//					mSmartVideo.stopVideo();
				}
			

			} else if (CameraSurfaceView.sceneResult[0] == 9) {
				// QRcode scene
				GLES20.glEnable(GLES20.GL_DEPTH_TEST);
				GLES20.glEnable(GLES20.GL_CULL_FACE);
				if(qrcodeModelFlag == QRCODE_MODLE_FLAG_CUBE)
				{
				String qrString = SmartNative.getQRCodeDecodeTxt();
				String qrType = SmartNative.getQRCodeDecodeType();
				qrString = qrString + " \n" + qrType;
				mCubeRenderer.draw(GLSmartCube3D.CUBE3D_TYPE_CUBE,
						porjectionMat, qrString);
//				mSmartVideo.stopVideo();
				}
				else
				{
//					if(!mSmartVideo.getPlayingFlag())
//						mSmartVideo.startVideo();
//					else
//					    mSmartVideo.draw(GlobalParams.SCREEN_WIDTH, GlobalParams.SCREEN_HEIGHT, porjectionMat);
				}
			} else {
//				mSmartVideo.stopVideo();
				mCubeRenderer.animationReset();
			}
		} else {
			mCubeRenderer.animationReset();
		}
	}

	public static final int FOOD_DISPLAY_HEALTH_INFO = 0;
	public static final int FOOD_DISPLAY_FLY = 1;
	public static final int FOOD_DISPLAY_FOG = 2;
	public static final int FOOD_MODLE_FLAG_VIDEO = 3;
	public static final int FOOD_MODLE_NUMBERS = 3;//total number of different models
	public static int foodModelFlag = FOOD_DISPLAY_HEALTH_INFO;
	
	public static final int QRCODE_MODLE_FLAG_CUBE = 0;
	public static final int QRCODE_MODLE_FLAG_VIDEO = 1;
	public static final int QRCODE_MODLE_NUMBERS = 1;//total number of different models
	public static int qrcodeModelFlag = QRCODE_MODLE_FLAG_CUBE;
}
