package com.lenovo.smartengine.gl.model;

import static android.opengl.GLES20.GL_TEXTURE0;
import static android.opengl.GLES20.GL_TEXTURE_2D;
import static android.opengl.GLES20.GL_TRIANGLE_FAN;
import static android.opengl.GLES20.GL_UNSIGNED_SHORT;
import static android.opengl.GLES20.glActiveTexture;
import static android.opengl.GLES20.glBindTexture;
import static android.opengl.GLES20.glDisableVertexAttribArray;
import static android.opengl.GLES20.glDrawElements;
import static android.opengl.GLES20.glGenTextures;
import static android.opengl.GLES20.glGetAttribLocation;
import static android.opengl.GLES20.glGetUniformLocation;
import static android.opengl.GLES20.glUniform1i;
import static android.opengl.GLES20.glUseProgram;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.FloatBuffer;
import java.nio.IntBuffer;
import java.nio.ShortBuffer;



import android.content.Context;
import android.graphics.Bitmap;
import android.opengl.GLES20;

import com.example.arframetest.R;
import com.lenovo.smartengine.util.OpenGLUtil;

public class GLSmartRGB2D {

	private int program_preview;
	private int uTextureUnitLocation,uTextureUnitLocation_preview;
	private int aTextureCoordinatesLocation_preview,aPositionLocation_preview;

	private Context mContext;
	
	private ShortBuffer cubeFaceData_1;
	private int[] texture = new int[1];	
	private FloatBuffer previewVerticesBuffer;
	private FloatBuffer previewTextureBuffer;
	
	public static IntBuffer bmpTempBuffer;
	private int bmpBufferWidth, bmpBufferHeight;
	
	private static final int BYTES_PER_SHORT = 2;
	private static final int BYTES_PER_FLOAT = 4;
	private final int COORDS_PER_VERTEX = 2;
	private final int vertexStride = COORDS_PER_VERTEX * 4; // 4 bytes per vertex
	private final short drawOrder[] = { 0, 1, 2, 0, 2, 3 }; // order to draw vertices
	private final float TexVertices[] = { // in counterclockwise order:
			1.0f, 0.0f, 
			1.0f, 1.0f, 
			0.0f, 1.0f, 
			0.0f, 0.0f 
		 };

	private final float squareVertices[] = { // in counterclockwise order:
			1.0f, 1.0f, 
			1.0f, -1.0f,
			-1.0f, -1.0f, 
			-1.0f, 1.0f
			};
	
	private short[] cube_faces_1 = {
			/* +z */3, 2, 1, 0,};
	
	private GLSmartRGB2D(Context context) {
		mContext = context;
		init();
	}

	public static GLSmartRGB2D Create(Context context) {
		return new GLSmartRGB2D(context);
	}

	private void init() {
		final String vertexShaderString_preview = OpenGLUtil.GLLoadFraomRawFile(/*"preview_vertex_shader.glsl"*/R.raw.rgb_vert_shad,mContext.getResources());
		final String fragmentShaderString_preview = OpenGLUtil.GLLoadFraomRawFile(/*"preview_fragment_shader.glsl"*/R.raw.rgb_frag_shad, mContext.getResources());
		
		
		program_preview = OpenGLUtil.GLCreateProgram(vertexShaderString_preview, fragmentShaderString_preview);
		
		//----get attribute and uniform variables in shader----------------
		aPositionLocation_preview = glGetAttribLocation(program_preview, "a_Position");
		aTextureCoordinatesLocation_preview =glGetAttribLocation(program_preview, "a_TextureCoordinates");

		uTextureUnitLocation_preview = glGetUniformLocation(program_preview, "u_TextureUnit");
		

		glGenTextures(1, texture, 0);   
		GLES20.glBindTexture(GLES20.GL_TEXTURE_2D, texture[0]);
		glTexParameter(); 
			
		cubeFaceData_1 = ByteBuffer.allocateDirect(cube_faces_1.length * BYTES_PER_SHORT).order(ByteOrder.nativeOrder()).asShortBuffer();
		cubeFaceData_1.put(cube_faces_1);
		cubeFaceData_1.position(0);
		
		previewVerticesBuffer = ByteBuffer.allocateDirect(squareVertices.length * BYTES_PER_FLOAT).order(ByteOrder.nativeOrder()).asFloatBuffer();
		previewVerticesBuffer.put(squareVertices);
		previewVerticesBuffer.position(0);
		
		previewTextureBuffer = ByteBuffer.allocateDirect(TexVertices.length * BYTES_PER_FLOAT).order(ByteOrder.nativeOrder()).asFloatBuffer();;
		previewTextureBuffer.put(TexVertices);
		previewTextureBuffer.position(0);
    	
	}
	
    /**
     * set Texture parameter
     */
    private void glTexParameter() {
		GLES20.glTexParameterf(GLES20.GL_TEXTURE_2D, GLES20.GL_TEXTURE_MIN_FILTER, GLES20.GL_LINEAR);
		GLES20.glTexParameterf(GLES20.GL_TEXTURE_2D, GLES20.GL_TEXTURE_MAG_FILTER, GLES20.GL_LINEAR);
		GLES20.glTexParameteri(GLES20.GL_TEXTURE_2D, GLES20.GL_TEXTURE_WRAP_S, GLES20.GL_CLAMP_TO_EDGE);
		GLES20.glTexParameteri(GLES20.GL_TEXTURE_2D, GLES20.GL_TEXTURE_WRAP_T, GLES20.GL_CLAMP_TO_EDGE);
		
	}
	
	public void setBitmap(Bitmap bm, int image_height, int image_width)
	{
		bmpBufferWidth = image_width;
		bmpBufferHeight = image_height;
        bmpTempBuffer = ByteBuffer.allocateDirect(bm.getWidth() * bm.getHeight() * 4).order(ByteOrder.nativeOrder()).asIntBuffer();
        bm.copyPixelsToBuffer(bmpTempBuffer);
        bmpTempBuffer.position(0);
	}
	
	public void draw() {
		glUseProgram(program_preview);    		

		GLES20.glEnableVertexAttribArray(aPositionLocation_preview);//向shader传输顶点坐标数组
		GLES20.glVertexAttribPointer(aPositionLocation_preview, COORDS_PER_VERTEX, GLES20.GL_FLOAT, false, vertexStride, previewVerticesBuffer);
		GLES20.glEnableVertexAttribArray(aTextureCoordinatesLocation_preview);//向shader传输用于渲染的纹理坐标数组
		GLES20.glVertexAttribPointer(aTextureCoordinatesLocation_preview, COORDS_PER_VERTEX, GLES20.GL_FLOAT, false, vertexStride, previewTextureBuffer);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture[0]);
		GLES20.glTexImage2D(GLES20.GL_TEXTURE_2D, 0, GLES20.GL_RGBA, bmpBufferWidth, bmpBufferHeight, 0,
				GLES20.GL_RGBA, GLES20.GL_UNSIGNED_BYTE, bmpTempBuffer);
		glUniform1i(uTextureUnitLocation, 0);
		
		glDrawElements(GL_TRIANGLE_FAN, 4, GL_UNSIGNED_SHORT , cubeFaceData_1.position(0*4));
		glDisableVertexAttribArray(aPositionLocation_preview);
		glDisableVertexAttribArray(aTextureCoordinatesLocation_preview);
	}
}
