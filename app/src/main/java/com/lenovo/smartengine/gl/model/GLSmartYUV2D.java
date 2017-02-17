package com.lenovo.smartengine.gl.model;

import java.nio.ByteBuffer;
import java.nio.FloatBuffer;
import java.util.Arrays;

import com.example.arframetest.R;
import android.content.Context;
import android.opengl.GLES20;

import com.lenovo.smartengine.util.GlobalParams;
import com.lenovo.smartengine.util.OpenGLUtil;

public class GLSmartYUV2D {
	int ATTRIB_VERTEX, ATTRIB_TEXTURE;
	
	float squareVertices[] = { -1.0f, -1.0f, 1.0f, -1.0f, -1.0f, 1.0f,
			1.0f, 1.0f, };
	float coordVertices[] = { 0.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
			0.0f, };


	private int[] texture = new int[6];
	private int _program;
	private int _tex_y_id;
	private int _tex_uv_id;
	private int tex_y;
	private int tex_u;

	private final int COORDS_PER_VERTEX = 2;
	private final int mVertexStride = COORDS_PER_VERTEX * 4; // 4 bytes per
	private Context mContext;
	
	public FloatBuffer mTVPreviewBuffer, mSVBuffer;
	
	static Object objectBufferLock = new Object();
//	private byte[] frameData = new byte[1280*720*3/2];
	public static byte[] frameData = new byte[1280*720*3/2];
	private static int frameHeight= 720;
	private static int frameWidth= 1280;

	private GLSmartYUV2D(Context context) {
		mContext = context;
		init();
	}

	public static GLSmartYUV2D Create(Context context) {
		return new GLSmartYUV2D(context);
	}

	private void init() {
		final String vertexShader = OpenGLUtil.GLLoadFraomRawFile(/*"preview_yuv.vert"*/ R.raw.yuv2rgb_vert_shad,mContext.getResources());
		final String fragmentShader = OpenGLUtil.GLLoadFraomRawFile(/*"preview_yuv.frag"*/R.raw.yuv2rgb_frag_shad, mContext.getResources());
		
		_program = OpenGLUtil.GLCreateProgram(vertexShader, fragmentShader);
		ATTRIB_VERTEX = GLES20.glGetAttribLocation(_program, "vPosition");

		ATTRIB_TEXTURE = GLES20.glGetAttribLocation(_program, "a_texCoord");
		tex_y = GLES20.glGetUniformLocation(_program, "texture0");
		tex_u = GLES20.glGetUniformLocation(_program, "texture1");

		GLES20.glGenTextures(6, texture, 0);
		_tex_y_id = texture[0];
		GLES20.glBindTexture(GLES20.GL_TEXTURE_2D, _tex_y_id);

		GLES20.glTexParameterf(GLES20.GL_TEXTURE_2D,
				GLES20.GL_TEXTURE_MIN_FILTER, GLES20.GL_LINEAR);
		GLES20.glTexParameterf(GLES20.GL_TEXTURE_2D,
				GLES20.GL_TEXTURE_MAG_FILTER, GLES20.GL_LINEAR);
		GLES20.glTexParameteri(GLES20.GL_TEXTURE_2D, GLES20.GL_TEXTURE_WRAP_S,
				GLES20.GL_CLAMP_TO_EDGE);
		GLES20.glTexParameteri(GLES20.GL_TEXTURE_2D, GLES20.GL_TEXTURE_WRAP_T,
				GLES20.GL_CLAMP_TO_EDGE);
		
		_tex_uv_id = texture[1];
		GLES20.glBindTexture(GLES20.GL_TEXTURE_2D, _tex_uv_id);

		GLES20.glTexParameterf(GLES20.GL_TEXTURE_2D,
				GLES20.GL_TEXTURE_MIN_FILTER, GLES20.GL_LINEAR);
		GLES20.glTexParameterf(GLES20.GL_TEXTURE_2D,
				GLES20.GL_TEXTURE_MAG_FILTER, GLES20.GL_LINEAR);
		GLES20.glTexParameteri(GLES20.GL_TEXTURE_2D, GLES20.GL_TEXTURE_WRAP_S,
				GLES20.GL_CLAMP_TO_EDGE);
		GLES20.glTexParameteri(GLES20.GL_TEXTURE_2D, GLES20.GL_TEXTURE_WRAP_T,
				GLES20.GL_CLAMP_TO_EDGE);

		mSVBuffer = OpenGLUtil.GLToFloatBuffer(squareVertices);
		mTVPreviewBuffer = OpenGLUtil.GLToFloatBuffer(coordVertices);
	}

	
	
	public static void setFrameData(byte[]data, int preview_width, int preview_height)
	{
//		synchronized (objectBufferLock) {
//		object.notify();

		frameData = Arrays.copyOf(data, preview_width*preview_height*3/2);
		frameHeight = preview_height;
		frameWidth = preview_width;
//		try {
//			objectBufferLock.wait();
//		} catch (InterruptedException e) {
//			// TODO Auto-generated catch block
//			e.printStackTrace();
//		}
//		}
	}
	public void draw() {
//		synchronized (objectBufferLock) {
		GLES20.glUseProgram(_program);
		
		mSVBuffer = OpenGLUtil.GLToFloatBuffer(squareVertices);
		
//		for(int i =0; i < frameHeight*frameWidth*3/2; i++)
//			frameData[i] = 90;
		ByteBuffer mBufferY = OpenGLUtil.GLToByteBuffer(frameData, 0, frameHeight*frameWidth);
		ByteBuffer mBufferUV = OpenGLUtil.GLToByteBuffer(frameData, frameHeight*frameWidth, frameHeight*frameWidth/2);
		
		GLES20.glActiveTexture(GLES20.GL_TEXTURE0);
		GLES20.glBindTexture(GLES20.GL_TEXTURE_2D, _tex_y_id);
		GLES20.glTexImage2D(GLES20.GL_TEXTURE_2D, 0, GLES20.GL_LUMINANCE, frameWidth, frameHeight, 0,
				GLES20.GL_LUMINANCE, GLES20.GL_UNSIGNED_BYTE, mBufferY);
		GLES20.glUniform1i(tex_y, 0);
		
		GLES20.glActiveTexture(GLES20.GL_TEXTURE1);
		GLES20.glBindTexture(GLES20.GL_TEXTURE_2D, _tex_uv_id);
		GLES20.glTexImage2D(GLES20.GL_TEXTURE_2D, 0, GLES20.GL_LUMINANCE_ALPHA, frameWidth/2, frameHeight/2, 0,
				GLES20.GL_LUMINANCE_ALPHA, GLES20.GL_UNSIGNED_BYTE, mBufferUV);
		GLES20.glUniform1i(tex_u, 1);
		
		GLES20.glVertexAttribPointer(ATTRIB_VERTEX, COORDS_PER_VERTEX,
		GLES20.GL_FLOAT, false, mVertexStride, mSVBuffer);
		GLES20.glEnableVertexAttribArray(ATTRIB_VERTEX);

		GLES20.glVertexAttribPointer(ATTRIB_TEXTURE, COORDS_PER_VERTEX,
		GLES20.GL_FLOAT, false, mVertexStride, mTVPreviewBuffer);
		GLES20.glEnableVertexAttribArray(ATTRIB_TEXTURE);

		GLES20.glDrawArrays(GLES20.GL_TRIANGLE_STRIP, 0, 4);
		GLES20.glFinish();
//		objectBufferLock.notify();
//		}
	}
}
