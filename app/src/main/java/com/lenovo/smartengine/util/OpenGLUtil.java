package com.lenovo.smartengine.util;

import java.io.ByteArrayOutputStream;
import java.io.InputStream;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.FloatBuffer;
import java.nio.IntBuffer;
import java.nio.ShortBuffer;

import android.content.res.Resources;
import android.opengl.GLES20;
import android.util.Log;

public class OpenGLUtil{
	
	private int[] mBufferedTextureIds;
	private int mCurrentTextureIndex = 0;
	private int mNextTextureIndex = 1;
	
	public FloatBuffer mTVFBOBuffer, mTVPreviewBuffer, mSVBuffer;
	public ShortBuffer mDrawOrderBuffer;

	public static final String TAG = "OpenGLMgr";

	public int[] getTextureBuffer()
	{
		return mBufferedTextureIds;
	}
	public void CreateTextureBuffer(int length, int width, int height){		
		mBufferedTextureIds = new int[length];
		for(int i = 0; i < length; i++)
		{
			mBufferedTextureIds[i] = OpenGLUtil.GLInitTextureNull(width, height);
		}
	}
	
//	public int GetCurrentTextureId(){
//		return mBufferedTextureIds[mCurrentTextureIndex];
//	}
	/** 
	 * get previous texture id
	 * @param i define how long between current frame,eg.1 :the latest stored frame
	 * @return 
	 */
	public int GetPreviousTextureId(int i){
		int tempId = mCurrentTextureIndex - i + 1;
		
		if(tempId < mBufferedTextureIds.length && tempId >= 0)
			tempId = tempId;
		else if(tempId > mBufferedTextureIds.length)
			tempId = mCurrentTextureIndex;//erro index input
		else if(tempId < 0){
			tempId = tempId+mBufferedTextureIds.length;
			if(tempId < mBufferedTextureIds.length && tempId >= 0){
				tempId = tempId;
			}else{
				tempId = mCurrentTextureIndex;// error index input
			}
		}
		
		return mBufferedTextureIds[tempId];
	}
	
	// 
	public int GetNextTextureId(){
//		int nextId = -1;
//		if(mCurrentTextureIndex < mBufferedTextureIds.length-1)
//			nextId = mCurrentTextureIndex + 1;
//		else
//			nextId = 0;
		return mBufferedTextureIds[mNextTextureIndex];
	}
	
	public void BufferIdNext(){
		if(mCurrentTextureIndex < mBufferedTextureIds.length-1)
			mCurrentTextureIndex = mCurrentTextureIndex + 1;
		else
			mCurrentTextureIndex = 0;
		
		if(mCurrentTextureIndex < mBufferedTextureIds.length-1)
			mNextTextureIndex = mCurrentTextureIndex + 1;
		else
			mNextTextureIndex = 0;
	}
	
	void SetSurfaceTexture() {

	}
	
	/**
	 * byte������ת��ΪByte��Buffer
	 * @param buffer
	 * @return Byte��Buffer
	 */
	public static ByteBuffer GLToByteBuffer(byte[] buffer) {
		ByteBuffer byteBuffer = ByteBuffer.allocateDirect(buffer.length);
		byteBuffer.order(ByteOrder.nativeOrder());
		byteBuffer.put(buffer);
		byteBuffer.position(0);
		return byteBuffer;
	}
	public static ByteBuffer GLToByteBuffer(byte[] buffer, int startIndex, int length) {
		ByteBuffer byteBuffer = ByteBuffer.allocateDirect(length);
		byteBuffer.order(ByteOrder.nativeOrder());
		byteBuffer.put(buffer, startIndex, length);
		byteBuffer.position(0);
		return byteBuffer;
	}
	/**
	 * float������ת��ΪFloat��Buffer
	 * @param buffer
	 * @return Float��Buffer
	 */
	public static FloatBuffer GLToFloatBuffer(float[] buffer) {
		FloatBuffer floatBuffer = null;
		ByteBuffer byteBuffer = ByteBuffer.allocateDirect(buffer.length * 4);
		byteBuffer.order(ByteOrder.nativeOrder());
		floatBuffer = byteBuffer.asFloatBuffer();
		floatBuffer.put(buffer);
		floatBuffer.position(0);
		return floatBuffer;
	}
	/**
	 * short������ת��ΪShort��Buffer
	 * @param buffer
	 * @return Short��Buffer
	 */
	public static ShortBuffer GLToShortBuffer(short[] buffer) {
		ShortBuffer shortBuffer = null;
		ByteBuffer byteBuffer = ByteBuffer.allocateDirect(buffer.length * 2);
		byteBuffer.order(ByteOrder.nativeOrder());
		shortBuffer = byteBuffer.asShortBuffer();
		shortBuffer.put(buffer);
		shortBuffer.position(0);
		return shortBuffer;
	}
	/**
	 * ��Assets�м����ļ�
	 * @param fileName �ļ����ƣ�*.sh�ļ���
	 * @param r
	 * @return
	 */
	public static String GLLoadFromAssetsFile(String fileName, Resources r) {
		String result = null;
		try {
			InputStream in = r.getAssets().open(fileName);
			int ch = 0;
			ByteArrayOutputStream baos = new ByteArrayOutputStream();
			while ((ch = in.read()) != -1) {
				baos.write(ch);
			}
			byte[] buff = baos.toByteArray();
			baos.close();
			in.close();
			result = new String(buff, "UTF-8");
			result = result.replaceAll("\\r\\n", "\n");
		} catch (Exception e) {
			e.printStackTrace();
		}
		return result;
	}
	
	public static String GLLoadFraomRawFile(int fileName, Resources r) {
		String result = null;
		try {
			InputStream in = r.openRawResource(fileName);
			int ch = 0;
			ByteArrayOutputStream baos = new ByteArrayOutputStream();
			while ((ch = in.read()) != -1) {
				baos.write(ch);
			}
			byte[] buff = baos.toByteArray();
			baos.close();
			in.close();
			result = new String(buff, "UTF-8");
			result = result.replaceAll("\\r\\n", "\n");
		} catch (Exception e) {
			e.printStackTrace();
		}
		return result;
	}
	/**
	 * ����Shader�������
	 * @param vertexSource ����shader����
	 * @param fragmentSource ƬԪshader����
	 * @return Program����
	 */
	public static int GLCreateProgram(String vertexSource, String fragmentSource) {
		int vertexShader = GLLoadShader(GLES20.GL_VERTEX_SHADER, vertexSource);
		if (vertexShader == 0) {
			return 0;
		}

		int pixelShader = GLLoadShader(GLES20.GL_FRAGMENT_SHADER, fragmentSource);
		if (pixelShader == 0) {
			return 0;
		}

		int program = GLES20.glCreateProgram();
		if (program != 0) {
			GLES20.glAttachShader(program, vertexShader);
			GLCheckGlError("glAttachShader");
			GLES20.glAttachShader(program, pixelShader);
			GLCheckGlError("glAttachShader");
			GLES20.glLinkProgram(program);
			int[] linkStatus = new int[1];
			GLES20.glGetProgramiv(program, GLES20.GL_LINK_STATUS, linkStatus, 0);
			if (linkStatus[0] != GLES20.GL_TRUE) {
				Log.e(TAG, "Could not link program: ");
				Log.e(TAG, GLES20.glGetProgramInfoLog(program));
				GLES20.glDeleteProgram(program);
				program = 0;
			}
		}
		return program;
	}
	/**
	 * ���ز�����shader����
	 * @param shaderType shader���ͣ�GL_VERTEX_SHADER��GL_FRAGMENT_SHADER
	 * @param source shader����
	 * @return shader����
	 */
	private static int GLLoadShader(int shaderType,  String source ) {
		int shader = GLES20.glCreateShader(shaderType);
		if (shader != 0) {
			GLES20.glShaderSource(shader, source);
			GLES20.glCompileShader(shader);
			int[] compiled = new int[1];
			GLES20.glGetShaderiv(shader, GLES20.GL_COMPILE_STATUS, compiled, 0);
			if (compiled[0] == 0) {
				Log.e(TAG, "Could not compile shader " + shaderType + ":");
				Log.e(TAG, GLES20.glGetShaderInfoLog(shader));
				GLES20.glDeleteShader(shader);
				shader = 0;
			}
		}
		return shader;
	}
	/**
	 * ������
	 * @param op
	 */
	public static void GLCheckGlError(String op) {
		int error;
		while ((error = GLES20.glGetError()) != GLES20.GL_NO_ERROR) {
			Log.e(TAG, op + ": glError " + error);
			throw new RuntimeException(op + ": glError " + error);
		}
	}
	/**
	 * ����FBO
	 * @return FBO����
	 */
	public static int GLGenFBO() {
		int[] fbo = new int[1];
        IntBuffer frameBuffer = IntBuffer.wrap(fbo);
		GLES20.glGenFramebuffers(fbo.length, frameBuffer);
		return fbo[0];
	}
	/**
	 * ����������
	 * @param 
	 * @return 
	 */
	public static int GLInitTextureNull(int width, int height) {
		int[] textures = new int[1];
		GLES20.glGenTextures(1, // ����������id������
				textures, // ����id������
				0 // ƫ����
		);
		int textureId = textures[0];
		GLES20.glBindTexture(GLES20.GL_TEXTURE_2D, textureId);

		GLES20.glTexParameterf(GLES20.GL_TEXTURE_2D,
				GLES20.GL_TEXTURE_MIN_FILTER, GLES20.GL_LINEAR);
		GLES20.glTexParameterf(GLES20.GL_TEXTURE_2D,
				GLES20.GL_TEXTURE_MAG_FILTER, GLES20.GL_LINEAR);
		GLES20.glTexParameteri(GLES20.GL_TEXTURE_2D, GLES20.GL_TEXTURE_WRAP_S,
				GLES20.GL_CLAMP_TO_EDGE);
		GLES20.glTexParameteri(GLES20.GL_TEXTURE_2D, GLES20.GL_TEXTURE_WRAP_T,
				GLES20.GL_CLAMP_TO_EDGE);

		GLES20.glTexImage2D(GLES20.GL_TEXTURE_2D, 0, GLES20.GL_RGBA, width,
				height, 0, GLES20.GL_RGBA, GLES20.GL_UNSIGNED_BYTE, null);

		return textureId;
	}
	public static int GLInitTextureNull(int width, int height, int innerType) {
		// ��������ID
		int[] textures = new int[1];
		GLES20.glGenTextures(1, // ����������id������
				textures, // ����id������
				0 // ƫ����
				);
		int textureId = textures[0];
		GLES20.glBindTexture(GLES20.GL_TEXTURE_2D, textureId);
		
		GLES20.glTexParameterf(GLES20.GL_TEXTURE_2D,
				GLES20.GL_TEXTURE_MIN_FILTER, GLES20.GL_LINEAR);
		GLES20.glTexParameterf(GLES20.GL_TEXTURE_2D,
				GLES20.GL_TEXTURE_MAG_FILTER, GLES20.GL_LINEAR);
		GLES20.glTexParameteri(GLES20.GL_TEXTURE_2D, GLES20.GL_TEXTURE_WRAP_S,
				GLES20.GL_CLAMP_TO_EDGE);
		GLES20.glTexParameteri(GLES20.GL_TEXTURE_2D, GLES20.GL_TEXTURE_WRAP_T,
				GLES20.GL_CLAMP_TO_EDGE);
		
		GLES20.glTexImage2D(GLES20.GL_TEXTURE_2D, 0, innerType, width,
				height, 0, GLES20.GL_RGBA, GLES20.GL_UNSIGNED_BYTE, null);
//		GLES20.glTexImage2D(GLES20.GL_TEXTURE_2D, 0, GLES20.GL_RGBA, width,
//				height, 0, GLES20.GL_RGBA, GLES20.GL_UNSIGNED_BYTE, null);
		
		return textureId;
	}
	/**
	 * read fbo data to int 
	 */
	public static int[] GLReadFboData(int matrixWith, int matriHeight) {
		int length = matrixWith * matriHeight * 4;

		ByteBuffer bufResultBufFinal = ByteBuffer.allocate(length); // ��ȡ����
		GLES20.glReadPixels(0, 0, matrixWith, matriHeight, GLES20.GL_RGBA,
				GLES20.GL_UNSIGNED_BYTE, bufResultBufFinal); // ��ȡ��Ȩ���������ĿǰΪ4ͨ��RGBA����
		GLES20.glFlush();

		byte[] resultFinalbuf = bufResultBufFinal.array();
		// ����ת���ɷǷ�����
		int[] resultFinal = new int[length];
		for (int i = 0; i < length; i++) {
			resultFinal[i] = ((int) resultFinalbuf[i]) & 0xFF;
		}
		return resultFinal;
	}

}