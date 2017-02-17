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

import android.content.Context;
import android.opengl.GLES20;
import android.util.Log;

import com.example.arframetest.R;
import com.google.vrtoolkit.cardboard.CardboardView;
import com.google.vrtoolkit.cardboard.EyeTransform;
import com.google.vrtoolkit.cardboard.HeadTransform;
import com.google.vrtoolkit.cardboard.Viewport;
import com.lenovo.smartengine.gl.model.GLSmartCube3D;
import com.lenovo.smartengine.gl.model.GLSmartPlugin;
import com.lenovo.smartengine.gl.model.GLSmartRGB2D;
import com.lenovo.smartengine.gl.model.GLSmartYUV2D;
import com.lenovo.smartengine.jni.SmartNative;
import com.lenovo.smartengine.util.GlobalParams;
import com.lenovo.smartengine.view.CameraSurfaceView;

public class SmartStereoRenderer implements CardboardView.StereoRenderer {

	private static final String TAG = "SmartSingleRenderer";

	private GLSmartYUV2D mPreviewRenderer;
	private GLSmartCube3D mCubeRenderer;
	private GLSmartPlugin glPlugin;
	public static GLSmartRGB2D mRGBRenderer;
	private final Context context;

	public SmartStereoRenderer(Context context) {
		this.context = context;
		glPlugin = new GLSmartPlugin(context);
	}

	public static final int FOOD_DISPLAY_HEALTH_INFO = 0;
	public static final int FOOD_DISPLAY_FLY = 1;
	public static final int FOOD_DISPLAY_FOG = 2;

	// public static boolean modelFlag = true;
	public static int modelFlag = FOOD_DISPLAY_HEALTH_INFO;
	public static int model_index = 0;

	@Override
	public void onDrawEye(EyeTransform arg0) {
		// TODO Auto-generated method stub
		GLES20.glClear(GLES20.GL_COLOR_BUFFER_BIT | GLES20.GL_DEPTH_BUFFER_BIT);
		GLES20.glClearColor(0.5f, 0f, 0.5f, 1f);
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
				if (modelFlag == FOOD_DISPLAY_FLY) {
//					glPlugin.SmartGLRenderPlugginOnDramFrame(
//							SmartNative.getGlProjectionMatrix(),
//							SmartNative.getExtrinsicMatrix(), 1);
				}

			} else if (CameraSurfaceView.sceneResult[0] == 9) {
				// QRcode scene
				GLES20.glEnable(GLES20.GL_DEPTH_TEST);
				GLES20.glEnable(GLES20.GL_CULL_FACE);
				String qrString = SmartNative.getQRCodeDecodeTxt();
				String qrType = SmartNative.getQRCodeDecodeType();
				qrString = qrString + " \n" + qrType;

				mCubeRenderer.draw(GLSmartCube3D.CUBE3D_TYPE_PYRAMID,
						porjectionMat, qrString);
			} else {
				mCubeRenderer.animationReset();
			}
		} else {
			mCubeRenderer.animationReset();
		}
	}

	@Override
	public void onFinishFrame(Viewport arg0) {
		// TODO Auto-generated method stub

	}

	@Override
	public void onNewFrame(HeadTransform arg0) {
		// TODO Auto-generated method stub

	}

	@Override
	public void onRendererShutdown() {
		// TODO Auto-generated method stub

	}

	@Override
	public void onSurfaceChanged(int width, int height) {
		glViewport(0, 0, width, height);
		glPlugin.SmartGLRenderPlugginOnSurfaceChanged(width, height);
	}

	@Override
	public void onSurfaceCreated(EGLConfig arg0) {
		// TODO Auto-generated method stub
		mPreviewRenderer = GLSmartYUV2D.Create(context);
		mCubeRenderer = GLSmartCube3D.Create(context);
		mRGBRenderer = GLSmartRGB2D.Create(context);

		glPlugin.SmartGLRenderPlugginOnSurfaceCreated(
				R.raw.ant_black_crunch_uv, R.raw.ant_black_uv);
	}

}
