package com.lenovo.smartengine.gl.model;

import static android.opengl.GLUtils.texImage2D;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.FloatBuffer;
import java.nio.IntBuffer;
import java.nio.ShortBuffer;




import com.example.arframetest.R;
import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Typeface;
import android.opengl.GLES20;
import android.text.Layout.Alignment;
import android.text.StaticLayout;
import android.text.TextPaint;

import com.lenovo.smartengine.util.OpenGLUtil;

public class GLSmartCube3D {
	
	public static final int CUBE3D_TYPE_CUBE            = 1;
	public static final int CUBE3D_TYPE_PYRAMID         = 2;
	public static final int CUBE3D_TYPE_TRANGULAR_PRISM = 3;
	
	//vertices coordinates: top surface of cube (animation)
	private float[] cube_vertices_4_2 = { 
			0.5f, 0.5f,  0.5f,   0.5f, -0.5f, 0.5f,    -0.5f, -0.5f, 0.5f,    -0.5f,0.5f, 0.5f
	};
	//vertices coordinates: top surface of cube (static)
	private float[] cube_vertices_4_3 = {
			0.5f, 0.5f,  0.5f,   0.5f, -0.5f, 0.5f,    -0.5f, -0.5f, 0.5f,    -0.5f,0.5f, 0.5f
	};
	//vertices coordinates: angular surface of cube (static)
	private float[] cube_vertices_4_4 = {
			0.25f, 0.25f,  0.25f,   0.25f, -0.25f, -0.25f,   -0.25f, -0.25f, -0.25f,    -0.25f, 0.25f, 0.25f
	};
	//vertices coordinates: trangular_prism
	private float[] cube_vertices_6_1 = {
			0.5f, 0.5f,  0.5f,   0.5f, -0.5f, -0.5f,
			-0.5f, -0.5f, -0.5f,    -0.5f,0.5f, 0.5f,
			0.5f, 0.5f,  -0.5f,   -0.5f, 0.5f, -0.5f
	};
	//vertices coordinates: pyramid (animation)
	private float[] cube_vertices_5_1 = {
			0.5f, 0.5f,  0.5f,   0.5f, -0.5f, 0.5f,
			-0.5f, -0.5f, 0.5f,    -0.5f,0.5f, 0.5f,
			0.0f, 0.0f,  -0.5f
	};
	//vertices coordinates: pyramid (static)
	private float[] cube_vertices_5_2 = {
			0.5f, 0.5f,  0.5f,   0.5f, -0.5f, 0.5f,
			-0.5f, -0.5f, 0.5f,    -0.5f,0.5f, 0.5f,
			0.0f, 0.0f,  -0.5f
	};
	//vertices coordinates: cube (animation)
	private float[] cube_vertices_8_1 = {
	/* +z */0.5f, 0.5f,  0.5f,   0.5f, -0.5f, 0.5f,   -0.5f, -0.5f, 0.5f,    -0.5f,0.5f, 0.5f,
	/* -z */0.5f, 0.5f, -0.5f,   0.5f, -0.5f,-0.5f,   -0.5f, -0.5f,-0.5f,    -0.5f, 0.5f, -0.5f };
	//vertices coordinates: cube (static)
	private float[] cube_vertices_8_2 = {
	/* +z */0.5f, 0.5f,  0.5f,   0.5f, -0.5f, 0.5f,   -0.5f, -0.5f, 0.5f,    -0.5f,0.5f, 0.5f,
	/* -z */0.5f, 0.5f, -0.5f,   0.5f, -0.5f,-0.5f,   -0.5f, -0.5f,-0.5f,    -0.5f, 0.5f, -0.5f };
	//texture draw order: surface to draw bmp texture
	private short[] cube_faces_1 = {
			3, 2, 1, 0,};
	//line draw order: rim of trangular_prism
	private short[] cube_lines_3 = {
			3, 2, 1, 0,
			4, 1, 2, 5,
			3, 0, 4, 5};
	//line draw order: rim of pyramid
	private short[] cube_lines_4 = {
			0, 1, 4,
			1, 2, 4,
			2, 3, 4, 
			3, 0, 4};
	//line draw order: rim of cube
	private short[] cube_lines_6 = {
			2, 3, 7, 6,
			0, 1, 5, 4,
			3, 0, 4, 7,
			1, 2, 6, 5,
			4, 5, 6, 7};
	//texture coordinates
	private float[] cube_texture_4_1 = { 1f, 0f,   1f, 1f,   0f, 1f,   0f, 0f
	};
	//animation rate
	private float ratio = 0.1f;
	//texture ID
	private int[] texture = new int[6];
	private int tex_blue_id_1;
	private int tex_blue_id_2;
	private int tex_red_id;
	private int tex_bmp_id;
	//Shader programe and param ID
	private int program;
	private int aPositionLocation;
	private int uMatrixLocation;
	private int uFlagLocation;	
	private int uTextureUnitLocation;
	private int uTextureUnitLocation1;
	private int aTextureCoordinatesLocation;	
	
	//Buffer for coordinates
	private FloatBuffer cubeTextureData_4_1;	
	private ShortBuffer cubeFaceData_1;
	
	private IntBuffer bmpBuffer;

	static Object objectBufferLock = new Object();
	private Context mContext;

	private GLSmartCube3D(Context context) {
		mContext = context;
		init();
	}

	public static GLSmartCube3D Create(Context context) {
		return new GLSmartCube3D(context);
	}

	/**
	 * init GL parameters(program,texture,coordinates,etc.)
	 */
	private void init() {

		//----create glProgram with vertex and fragment ----------------
		final String vertexShader = OpenGLUtil.GLLoadFraomRawFile(/*"simple_vertex_shader.glsl"*/ R.raw.simple_vertex_shader,mContext.getResources());
		final String fragmentShader = OpenGLUtil.GLLoadFraomRawFile(/*"simple_fragment_shader.glsl"*/R.raw.simple_fragment_shader, mContext.getResources());
		program = OpenGLUtil.GLCreateProgram(vertexShader, fragmentShader);
		
		//----get attribute and uniform variables in shader----------------		
		aPositionLocation = GLES20.glGetAttribLocation(program, "a_Position");
		aTextureCoordinatesLocation =GLES20.glGetAttribLocation(program, "a_TextureCoordinates");
		
		uMatrixLocation = GLES20.glGetUniformLocation(program, "u_Matrix");
		uFlagLocation = GLES20.glGetUniformLocation(program, "Flag");
		uTextureUnitLocation = GLES20.glGetUniformLocation(program, "u_TextureUnit");
		uTextureUnitLocation1 = GLES20.glGetUniformLocation(program, "u_TextureUnit1");	
		
		
		//----init textures----------------	
		GLES20.glGenTextures(4, texture, 0);    
		
		tex_blue_id_1 = texture[0];
        GLES20.glBindTexture(GLES20.GL_TEXTURE_2D, tex_blue_id_1);  // 3-锟斤拷锟斤拷opengl锟斤拷应锟斤拷应锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷  
        glTexParameter();
        glTexColor(Color.BLUE);
        
        tex_blue_id_2 = texture[1];
        GLES20.glBindTexture(GLES20.GL_TEXTURE_2D, tex_blue_id_2);  // 3-锟斤拷锟斤拷opengl锟斤拷应锟斤拷应锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷  
        glTexParameter();
        glTexColor(Color.BLUE);


        tex_red_id = texture[2];
		GLES20.glBindTexture(GLES20.GL_TEXTURE_2D, tex_red_id);  
        glTexParameter();
        glTexColor(Color.RED);

        
//      tex_bmp_id = texture[3];
//      GLES20.glBindTexture(GLES20.GL_TEXTURE_2D, tex_bmp_id);  
//      glTexParameter();      
//      Bitmap bitmap1 = getBitmap(mContext, R.drawable.wall);   
//      texImage2D(GLES20.GL_TEXTURE_2D, 0, bitmap1, 0); 
//      bitmap1.recycle();

		//----set vertices and texture coordinate---------------
		cubeTextureData_4_1 = OpenGLUtil.GLToFloatBuffer(cube_texture_4_1);//
		cubeFaceData_1 = OpenGLUtil.GLToShortBuffer(cube_faces_1);//
	}

	/**
	 * draw 3D cube model
	 * @param type          model type
	 * @param porjectionMat model projection Mat
	 * @param displayMsg    message to display
	 */
	public void draw(int type, float[] porjectionMat, String displayMsg) {
//		synchronized (objectBufferLock) {
  
		switch(type){
			case CUBE3D_TYPE_CUBE:{
				animationInit(type, cube_vertices_8_2, cube_vertices_8_1);
				animationStart(4, cube_vertices_8_1, cube_vertices_4_2, cube_vertices_4_3, cube_lines_6, porjectionMat, displayMsg);
				break;
			}
			case CUBE3D_TYPE_PYRAMID:{
				animationInit(type, cube_vertices_5_2, cube_vertices_5_1);
				animationStart(3, cube_vertices_5_1, cube_vertices_4_2, cube_vertices_4_3, cube_lines_4, porjectionMat, displayMsg);
				break;	
			}
			case CUBE3D_TYPE_TRANGULAR_PRISM:
				model3DStart(4, cube_vertices_6_1, cube_vertices_4_4, cube_lines_3, porjectionMat, displayMsg);
				break;
			default:
				break;		
		}
//		objectBufferLock.notify();
//		}
	}
	
    /**
     * Reset animation parameter
     */
    public void animationReset() {
		ratio = 0.1f;
	}
    
    /**
     * Init animation parameters
     * @param type         cube type
     * @param verticesSrc  vertices coordinate for default
     * @param verticesDst  vertices coordinate for animation
     */
    private void animationInit(int type, float[] verticesSrc, float[] verticesDst) {
		//立方体生长
    	float temp; 
		if(ratio >= 1){
			ratio = 1.0f;
			for(int i = 0; i < verticesDst.length; i++){
				verticesDst[i] = verticesSrc[i];
			}
		}else{
			ratio += 0.05f;
		}
		
		if(type == CUBE3D_TYPE_CUBE){
			temp = 1.0f;
		}else{
			temp = ratio;
		}
		//立方体顶面设置（Z坐标变化，XY不变，确保生长动画）
		for(int i = 0; i < cube_vertices_4_2.length; i++){
			if(i % 3 < 2){    			
				verticesDst[i] = verticesSrc[i]*temp;
				if(ratio < 1){		
					cube_vertices_4_2[i] = verticesSrc[i]*temp;
				}
			}else{
				verticesDst[i] = ratio - verticesSrc[i];
				if(ratio < 1){			
					cube_vertices_4_2[i] = ratio - verticesSrc[i];
				}				
			}
		}
	}
    
    /**
     * start animation
     * @param count         stride of cubelinedata
     * @param verticesSrc   vertices coordinate for animation
     * @param verticesDst1  vertices coordinate for texture (animation)
     * @param verticesDst2  vertices coordinate for texture (static)
     * @param linedataSrc   cubelinedata of draw order
     * @param porjectionMat model projection Mat
     * @param displayMsg    message to display
     */
    private void animationStart(int count, float[] verticesSrc,float[] verticesDst1,float[] verticesDst2, short[] linedataSrc, float[] porjectionMat, String displayMsg) {
		//绘制RGB纹理（立体和bitmap）
		GLES20.glUseProgram(program);
    	//绘制立方体的边框
    	FloatBuffer cubeVertexData = OpenGLUtil.GLToFloatBuffer(verticesSrc);
    	ShortBuffer cubeLineData = OpenGLUtil.GLToShortBuffer(linedataSrc);
    	GLES20.glEnableVertexAttribArray(aPositionLocation);
    	GLES20.glVertexAttribPointer(aPositionLocation, 3, GLES20.GL_FLOAT ,false , 0 , cubeVertexData);
		GLES20.glUniform1f(uFlagLocation, 1.0f);
		
		GLES20.glUniformMatrix4fv(uMatrixLocation, 1, false, porjectionMat, 0);
		for (int i = 0; i < linedataSrc.length/count; i++) {
			GLES20.glDrawElements(GLES20.GL_LINE_LOOP, count, GLES20.GL_UNSIGNED_SHORT, cubeLineData.position(i*count));
	    }

		GLES20.glEnable(GLES20.GL_BLEND);
		GLES20.glBlendFunc(GLES20.GL_ONE, GLES20.GL_ONE);
		
		//绘制立方体六个面（纯色）
		GLES20.glEnableVertexAttribArray(aPositionLocation);
		GLES20.glVertexAttribPointer(aPositionLocation, 3, GLES20.GL_FLOAT ,false , 0 , cubeVertexData);
		GLES20.glEnableVertexAttribArray(aTextureCoordinatesLocation); 
		GLES20.glVertexAttribPointer( aTextureCoordinatesLocation, 2, GLES20.GL_FLOAT, false, 0 , cubeTextureData_4_1);
		GLES20.glUniform1f(uFlagLocation, 0.0f);
		GLES20.glActiveTexture(GLES20.GL_TEXTURE2);
		GLES20.glBindTexture(GLES20.GL_TEXTURE_2D, tex_red_id);  
		GLES20.glUniform1i(uTextureUnitLocation, 2);				
				
		for (int i = 0; i < linedataSrc.length/count; i++) {
			GLES20.glDrawElements(GLES20.GL_TRIANGLE_FAN, count, GLES20.GL_UNSIGNED_SHORT, cubeLineData.position(i*count));
	    }		
		
		if(ratio < 1.0f){
			//生长到顶峰前绘制纯色纹理
			cubeVertexData.put(verticesDst1);
			cubeVertexData.position(0);
    		GLES20.glEnableVertexAttribArray(aPositionLocation);
    		GLES20.glVertexAttribPointer(aPositionLocation, 3, GLES20.GL_FLOAT ,false , 0 , cubeVertexData);
    		GLES20.glEnableVertexAttribArray(aTextureCoordinatesLocation); 
    		GLES20.glVertexAttribPointer( aTextureCoordinatesLocation, 2, GLES20.GL_FLOAT, false, 0 , cubeTextureData_4_1);
    		GLES20.glUniform1f(uFlagLocation, 0.0f);
    		GLES20.glActiveTexture(GLES20.GL_TEXTURE2);
    		GLES20.glBindTexture(GLES20.GL_TEXTURE_2D, tex_blue_id_1);  
    		GLES20.glUniform1i(uTextureUnitLocation, 2);				
    				
    		GLES20.glDrawElements(GLES20.GL_TRIANGLE_FAN, 4, GLES20.GL_UNSIGNED_SHORT , cubeFaceData_1.position(0*4));
		}else{
			//生长到峰值后绘制图片
			bmpBuffer = getBitmap(256, 256, displayMsg);
			
			cubeVertexData.put(verticesDst2);
			cubeVertexData.position(0);
			GLES20.glEnableVertexAttribArray(aPositionLocation);
			GLES20.glVertexAttribPointer(aPositionLocation, 3, GLES20.GL_FLOAT ,false , 0 , cubeVertexData);			
			GLES20.glEnableVertexAttribArray(aTextureCoordinatesLocation); 
			GLES20.glVertexAttribPointer( aTextureCoordinatesLocation, 2, GLES20.GL_FLOAT, false, 0 , cubeTextureData_4_1);
			GLES20.glUniform1f(uFlagLocation, 0.0f);
			
			GLES20.glActiveTexture(GLES20.GL_TEXTURE2);
			GLES20.glBindTexture(GLES20.GL_TEXTURE_2D, tex_blue_id_2);  
			GLES20.glUniform1i(uTextureUnitLocation, 2);
			GLES20.glTexSubImage2D(GLES20.GL_TEXTURE_2D, 0, 0, 0, 256, 256, GLES20.GL_RGBA, GLES20.GL_UNSIGNED_BYTE, bmpBuffer);
			
			GLES20.glUniformMatrix4fv(uMatrixLocation, 1, false, porjectionMat, 0);
			
			GLES20.glDrawElements(GLES20.GL_TRIANGLE_FAN, 4, GLES20.GL_UNSIGNED_SHORT , cubeFaceData_1.position(0*4));

		}

		GLES20.glDisable(GLES20.GL_BLEND);
		GLES20.glDisableVertexAttribArray(aPositionLocation);
		GLES20.glDisableVertexAttribArray(aTextureCoordinatesLocation);
	}
    
    /**
     * Draw static 3D model
     * @param count         stride of cubelinedata
     * @param verticesSrc   vertices coordinate (static)
     * @param verticesDst   vertices coordinate for texture (static)
     * @param linedataSrc   cubelinedata of draw order
     * @param porjectionMat model projection Mat
     * @param displayMsg    message to display
     */
    private void model3DStart(int count, float[] verticesSrc, float[] verticesDst, short[] linedataSrc, float[] porjectionMat, String displayMsg) {

		//绘制RGB纹理（立体和bitmap）
		GLES20.glUseProgram(program); 
		FloatBuffer cubeVertexData = OpenGLUtil.GLToFloatBuffer(verticesSrc);
		ShortBuffer cubeLineData = OpenGLUtil.GLToShortBuffer(linedataSrc);
		GLES20.glEnableVertexAttribArray(aPositionLocation);
		GLES20.glVertexAttribPointer(aPositionLocation, 3, GLES20.GL_FLOAT ,false , 0 , cubeVertexData);
		GLES20.glUniform1f(uFlagLocation, 1.0f);
		
		for (int i = 0; i < linedataSrc.length/count; i++) {
			GLES20.glDrawElements(GLES20.GL_LINE_LOOP, count, GLES20.GL_UNSIGNED_SHORT, cubeLineData.position(i*count));
		}
		
		GLES20.glEnable(GLES20.GL_BLEND);
		GLES20.glBlendFunc(GLES20.GL_ONE, GLES20.GL_ONE);		
		
		bmpBuffer = getBitmap(256, 256, displayMsg); 	
		
		cubeVertexData.put(verticesDst);
		cubeVertexData.position(0);
		GLES20.glEnableVertexAttribArray(aPositionLocation);
		GLES20.glVertexAttribPointer(aPositionLocation, 3, GLES20.GL_FLOAT ,false , 0 , cubeVertexData);			
		GLES20.glEnableVertexAttribArray(aTextureCoordinatesLocation); 
		GLES20.glVertexAttribPointer( aTextureCoordinatesLocation, 2, GLES20.GL_FLOAT, false, 0 , cubeTextureData_4_1);
		GLES20.glUniform1f(uFlagLocation, 0.0f);
		
		GLES20.glActiveTexture(GLES20.GL_TEXTURE2);
		GLES20.glBindTexture(GLES20.GL_TEXTURE_2D, tex_blue_id_2);  
		GLES20.glUniform1i(uTextureUnitLocation, 2);
		GLES20.glTexSubImage2D(GLES20.GL_TEXTURE_2D, 0, 0, 0, 256, 256, GLES20.GL_RGBA, GLES20.GL_UNSIGNED_BYTE, bmpBuffer);
		
		GLES20.glUniformMatrix4fv(uMatrixLocation, 1, false, porjectionMat, 0);
		
		GLES20.glDrawElements(GLES20.GL_TRIANGLE_FAN, 4, GLES20.GL_UNSIGNED_SHORT , cubeFaceData_1.position(0*4));
		
		
		GLES20.glDisable(GLES20.GL_BLEND);
		GLES20.glDisableVertexAttribArray(aPositionLocation);
		GLES20.glDisableVertexAttribArray(aTextureCoordinatesLocation);	
	}
    /**
     * set Texture parameter
     */
    private void glTexParameter() {
		GLES20.glTexParameterf(GLES20.GL_TEXTURE_2D, GLES20.GL_TEXTURE_MIN_FILTER, GLES20.GL_LINEAR);
		GLES20.glTexParameterf(GLES20.GL_TEXTURE_2D, GLES20.GL_TEXTURE_MAG_FILTER, GLES20.GL_LINEAR);
		GLES20.glTexParameteri(GLES20.GL_TEXTURE_2D, GLES20.GL_TEXTURE_WRAP_S, GLES20.GL_CLAMP_TO_EDGE);
		GLES20.glTexParameteri(GLES20.GL_TEXTURE_2D, GLES20.GL_TEXTURE_WRAP_T, GLES20.GL_CLAMP_TO_EDGE);
//		GLES20.glTexParameteri(GLES20.GL_TEXTURE_2D, GLES20.GL_TEXTURE_WRAP_S, GLES20.GL_CLAMP_TO_EDGE);
//		GLES20.glTexParameteri(GLES20.GL_TEXTURE_2D, GLES20.GL_TEXTURE_WRAP_T, GLES20.GL_CLAMP_TO_EDGE);		
	}
    
    /**
     * set Texture color
     * @param color type of color 
     */
    private void glTexColor(int color) {
        Bitmap bmp = Bitmap.createBitmap(256, 256, Bitmap.Config.ARGB_8888);
        Canvas canvasTemp = new Canvas(bmp);
        canvasTemp.drawColor(color);
        texImage2D(GLES20.GL_TEXTURE_2D, 0, bmp, 0); 
        bmp.recycle();		
	}
    
    /**
     * get BMP buffer data
     * @param w             width of BMP
     * @param h			    height of BMP
     * @param paramDisplay  information to display
     * @return
     */
    private IntBuffer getBitmap(int w, int h, String paramDisplay) {
    	
    	Bitmap bmp = Bitmap.createBitmap(256, 256, Bitmap.Config.ARGB_8888);
        Canvas canvasTemp = new Canvas(bmp);
        canvasTemp.drawColor(Color.BLUE);
        
        TextPaint textPaint = new TextPaint();
        String familyName = "Times New Roman";
        Typeface font = Typeface.create(familyName, Typeface.BOLD);
        textPaint.setColor(Color.GREEN);
        textPaint.setTypeface(font);
        textPaint.setTextSize(30.0F);

        StaticLayout layout = new StaticLayout(paramDisplay,textPaint,256,Alignment.ALIGN_NORMAL,1.0F,0.0F,true);
        canvasTemp.translate(0,60);
        layout.draw(canvasTemp);      
        
        //----使锟斤拷IntBuffer锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷谢锟斤拷锟斤拷锟�-----------
        IntBuffer bmpTempBuffer = ByteBuffer.allocateDirect(bmp.getWidth() * bmp.getHeight() * 4).order(ByteOrder.nativeOrder()).asIntBuffer();
        bmp.copyPixelsToBuffer(bmpTempBuffer);
        bmpTempBuffer.position(0);  //确锟斤拷锟斤拷确锟斤拷锟斤拷锟斤拷锟斤拷始位锟矫ｏ拷锟斤拷止偏色
        
        return bmpTempBuffer;
    }

    /**
     * get BMP file
     * @param context
     * @param resId
     * @return
     */
    private Bitmap getBitmap(Context context,int resId)
    {
    	//getBitmap by decodeResources()
    	BitmapFactory.Options options = new BitmapFactory.Options();
    	options.inScaled = false;
    	return BitmapFactory.decodeResource(context.getResources(), resId, options);
    }	
}