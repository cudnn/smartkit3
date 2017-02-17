package com.lenovo.smartengine.gl.model;

import java.nio.FloatBuffer;
import java.nio.ShortBuffer;


import javax.microedition.khronos.opengles.GL10;

import com.example.arframetest.R;
import com.lenovo.smartengine.video.decoder.DecoderThread;
import com.lenovo.smartengine.video.decoder.IVCGLLib;
import com.lenovo.smartengine.video.decoder.SpeedController;

import android.app.Activity;
import android.content.Context;
import android.content.res.AssetFileDescriptor;
import android.graphics.SurfaceTexture;
import android.opengl.GLES11Ext;
import android.opengl.GLES20;
import android.util.Log;
import android.view.Surface;

public class GLSmartVideo {
	private static final String TAG = "GLSmartVideo";
//	int screenWidth, screenHeight;
	DecoderThread decoder; //done on a decoder thread to avoid block in UI
	
	//a flag to notify the renderer to start drawing frames once the decoder is ready
	//TODO: add a handler in renderer to handle this message.
	boolean playingFlag;
	
	Activity mContext;
	
	private int textureId;
	SurfaceTexture mSurfaceTexture;
	
	private String vertShader;
	private String fragShader_Pre;
	private int programHandle;
	private int mPositionHandle;
	private int mTextureCoordHandle;
	private int uMatrixLocation;
	
	FloatBuffer verticesBuffer, textureVerticesPreviewBuffer;
	private ShortBuffer drawListBuffer;
	
	// number of coordinates per vertex in this array
	private final int COORDS_PER_VERTEX = 3;
	
	private final int vertexStride = COORDS_PER_VERTEX * 4; // 4 bytes per vertex
	
//	private float squareVertices[] = { // in counterclockwise order:
//			-0.89f, 0.5f, -0.5f,
//			-0.89f, -0.5f, -0.5f,
//			0.89f, -0.5f, -0.5f,
//			0.89f, 0.5f, -0.5f
//		 };
	private float squareVertices[] = { // in counterclockwise order:
			-0.5f, 0.5f, -0.5f,
			-0.5f, -0.5f, -0.5f,
			0.5f, -0.5f, -0.5f,
			0.5f, 0.5f, -0.5f
	};
//	private final float squareVertices[] = { // in counterclockwise order:
//			-1.0f, 1.0f, 
//			-1.0f, -1.0f, 
//			1.0f, -1.0f, 
//			1.0f, 1.0f
//	};
	private final float textureVerticesPreview[] = { // in counterclockwise order:
			0.0f, 1.0f, 0.0f,
			1.0f, 1.0f,  0.0f,
			1.0f, 0.0f,  0.0f,
			0.0f, 0.0f,  0.0f
		 };
	private final short drawOrder[] = { 0, 1, 2, 0, 2, 3 }; // order to draw vertices

	private GLSmartVideo(Context context) {
		mContext = (Activity)context;
		init();
	}

	public static GLSmartVideo Create(Context context) {
		return new GLSmartVideo(context);
	}
	
	private void init() {
		verticesBuffer = IVCGLLib.glToFloatBuffer(squareVertices);
		textureVerticesPreviewBuffer = IVCGLLib
				.glToFloatBuffer(textureVerticesPreview);
		drawListBuffer = IVCGLLib.glToShortBuffer(drawOrder);
    	
//		vertShader = IVCGLLib.loadFromAssetsFile("IVC_VShader_Preview.sh", mContext.getResources());
//		fragShader_Pre = IVCGLLib.loadFromAssetsFile("IVC_FShader_Preview.sh", mContext.getResources());
		
		vertShader = IVCGLLib.GLLoadFraomRawFile(R.raw.video_vert_shad, mContext.getResources());
		fragShader_Pre = IVCGLLib.GLLoadFraomRawFile(R.raw.video_frag_shad, mContext.getResources());

		programHandle = IVCGLLib.glCreateProgram(vertShader, fragShader_Pre);
    	
    	mPositionHandle = GLES20.glGetAttribLocation(programHandle, "position");
    	mTextureCoordHandle = GLES20.glGetAttribLocation(programHandle, "inputTextureCoordinate");	
    	uMatrixLocation = GLES20.glGetUniformLocation(programHandle, "u_Matrix");
		
    	createSurfaceTexture();
		playingFlag = false;
	}
	
	public void draw(int screenWidth, int screenHeight, float[] porjectionMat)
	{
		if (playingFlag) {
//			GLES20.glClear(GLES20.GL_COLOR_BUFFER_BIT | GLES20.GL_DEPTH_BUFFER_BIT);		
			
			mSurfaceTexture.updateTexImage();
			
			GLES20.glUseProgram(programHandle);

			GLES20.glEnableVertexAttribArray(mPositionHandle);
			GLES20.glVertexAttribPointer(mPositionHandle, COORDS_PER_VERTEX, GLES20.GL_FLOAT, false, vertexStride, verticesBuffer);
			GLES20.glEnableVertexAttribArray(mTextureCoordHandle);
			GLES20.glVertexAttribPointer(mTextureCoordHandle, COORDS_PER_VERTEX,GLES20.GL_FLOAT, false, vertexStride, textureVerticesPreviewBuffer);
			
			
			GLES20.glActiveTexture(GLES20.GL_TEXTURE0);
			GLES20.glBindTexture(GLES11Ext.GL_TEXTURE_EXTERNAL_OES, textureId);			
			GLES20.glUniform1i(GLES20.glGetUniformLocation(programHandle, "sampler2d"), 0);
			GLES20.glUniformMatrix4fv(uMatrixLocation, 1, false, porjectionMat, 0);
   		    	
			IVCGLLib.glUseFBO(0, 0, screenWidth, screenHeight, false, 0, 0);
			GLES20.glDrawElements(GLES20.GL_TRIANGLES, drawOrder.length, GLES20.GL_UNSIGNED_SHORT, drawListBuffer);
			IVCGLLib.glCheckGlError("glDrawElements");	
			
			GLES20.glDisableVertexAttribArray(mPositionHandle);
			GLES20.glDisableVertexAttribArray(mTextureCoordHandle);
		}
	}
	
	public SurfaceTexture createSurfaceTexture() {
		textureId = createVideoTexture();
		mSurfaceTexture = new SurfaceTexture(textureId);
		return mSurfaceTexture;
	}
    private int createVideoTexture() {
        int[] texture = new int[1];

        GLES20.glGenTextures(1,texture, 0);
        GLES20.glBindTexture(GLES11Ext.GL_TEXTURE_EXTERNAL_OES, texture[0]);
        GLES20.glTexParameterf(GLES11Ext.GL_TEXTURE_EXTERNAL_OES,
             GL10.GL_TEXTURE_MIN_FILTER,GL10.GL_LINEAR);
        GLES20.glTexParameterf(GLES11Ext.GL_TEXTURE_EXTERNAL_OES,
             GL10.GL_TEXTURE_MAG_FILTER, GL10.GL_LINEAR);
        GLES20.glTexParameteri(GLES11Ext.GL_TEXTURE_EXTERNAL_OES,
             GL10.GL_TEXTURE_WRAP_S, GL10.GL_CLAMP_TO_EDGE);
        GLES20.glTexParameteri(GLES11Ext.GL_TEXTURE_EXTERNAL_OES,
             GL10.GL_TEXTURE_WRAP_T, GL10.GL_CLAMP_TO_EDGE);

        return texture[0];
    }
    
    void initTexture() {
    	verticesBuffer = IVCGLLib.glToFloatBuffer(squareVertices);
		textureVerticesPreviewBuffer = IVCGLLib
				.glToFloatBuffer(textureVerticesPreview);
		drawListBuffer = IVCGLLib.glToShortBuffer(drawOrder);
    	
//		vertShader = IVCGLLib.loadFromAssetsFile("IVC_VShader_Preview.sh", mContext.getResources());
//		fragShader_Pre = IVCGLLib.loadFromAssetsFile("IVC_FShader_Preview.sh", mContext.getResources());
		
		vertShader = IVCGLLib.GLLoadFraomRawFile(R.raw.video_vert_shad, mContext.getResources());
		fragShader_Pre = IVCGLLib.GLLoadFraomRawFile(R.raw.video_frag_shad, mContext.getResources());

		programHandle = IVCGLLib.glCreateProgram(vertShader, fragShader_Pre);
    	
    	mPositionHandle = GLES20.glGetAttribLocation(programHandle, "position");
    	mTextureCoordHandle = GLES20.glGetAttribLocation(programHandle, "inputTextureCoordinate");	
    	
    	
    }
    
    void release() {
    	Log.d(TAG, "deleting program " + programHandle);
        GLES20.glDeleteProgram(programHandle);
        programHandle = -1;
        
        Log.d(TAG, "releasing SurfaceTexture");
        if (mSurfaceTexture != null) {
        	mSurfaceTexture.release();
        	mSurfaceTexture = null;
        }        
    }
	
    
    //play video start
  	//prepare before playing every video.
  	public void prepareVideo() {
  		AssetFileDescriptor afd;
  		Surface surface;
  		SpeedController controller;
  		//read video from res/raw/
  	   	afd = mContext.getResources().openRawResourceFd(R.raw.zootopia);

//  		surfaceTexture = glSurfaceView.getSurfaceTexture();//get surfacetexture created in renderer
  		
  		//Register a callback when a new frame is available to the SurfaceTexture
//  	    mSurfaceTexture.setOnFrameAvailableListener(this);
  			
  		surface = new Surface(mSurfaceTexture);//get the surface to be used for decoding output
  			
  		controller = new SpeedController();
  		decoder = new DecoderThread(afd, surface, controller);//initialize all the decoding stuff here.
  	}
  	
  	public void startVideo() {
  		if (decoder != null) stopVideo();

  		prepareVideo();
  		decoder.startPlaying();//start the video decoding thread here
  			
  		playingFlag = true;
  		
		int vh = decoder.getVideoHeight();
		int vw = decoder.getVideoWidth();
		float ratio = (float)vw / (float)vh;
		
//		squareVertices[] = { // in counterclockwise order:
//				-0.5f, 0.5f, 
//				-0.5f, -0.5f, 
//				0.5f, -0.5f, 
//				0.5f, 0.5f
//			 };
		squareVertices[0] = -0.5f * ratio;
		squareVertices[3] = -0.5f * ratio;
		squareVertices[6] = 0.5f * ratio;
		squareVertices[9] = 0.5f * ratio;
		verticesBuffer = IVCGLLib.glToFloatBuffer(squareVertices);
//		squareVertices[0] = -0.89f ;
//		squareVertices[3] = -0.89f ;
//		squareVertices[6] = 0.89f ;
//		squareVertices[9] = 0.89f ;
//		 Log.d(TAG, "video ratio dd" + ratio);
  	}
  	
  	public void stopVideo() {
  		if (decoder != null) {
  			decoder.stopPlaying();//delete the thread
  			decoder = null;
  			playingFlag = false;
  		}
  	
  	}
  	
	public boolean getPlayingFlag() {
		// TODO Auto-generated method stub
		return playingFlag;
	}
	//playing video end
}
