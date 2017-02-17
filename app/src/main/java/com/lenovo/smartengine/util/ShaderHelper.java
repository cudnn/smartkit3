package com.lenovo.smartengine.util;

import static android.opengl.GLES20.GL_FRAGMENT_SHADER;
import static android.opengl.GLES20.GL_VERTEX_SHADER;
import static android.opengl.GLES20.glAttachShader;
import static android.opengl.GLES20.glCompileShader;
import static android.opengl.GLES20.glCreateProgram;
import static android.opengl.GLES20.glCreateShader;
import static android.opengl.GLES20.glLinkProgram;
import static android.opengl.GLES20.glShaderSource;
import android.util.Log;
 
public class ShaderHelper {
	
	public static int compileVertexShader(String shaderCode){
		return compileShader(GL_VERTEX_SHADER, shaderCode);
	}
	
	public static int compileFragmentShader(String shaderCode){
		return compileShader(GL_FRAGMENT_SHADER, shaderCode);
	}
	
	private static int compileShader( int type , String shaderCode ){
		
		final int shadeObjectId = glCreateShader(type);  //1-����һ����ɫ������
		
		if (shadeObjectId == 0) {
			Log.w("ShaderHelper", "Could not create new shader.");
			return 0;
		}
		
		glShaderSource(shadeObjectId, shaderCode); // 2- ����ɫ��Դ�� �ϴ� ����ɫ��������
		glCompileShader(shadeObjectId); // 3- ������ɫ��
		
		return shadeObjectId;
	}
	
	public static int linkProgram( int vertexShaderId , int fragmentShaderId ){
		final int programObjectId = glCreateProgram(); // 4-�½� ���� ����
		
		glAttachShader(programObjectId, vertexShaderId); // 5- �ڳ����� ���� ������ɫ��
		glAttachShader(programObjectId, fragmentShaderId); //6- �ڳ����м���Ƭ����ɫ��
		
		glLinkProgram(programObjectId); //7- ���ӳ���
		
		return programObjectId;

	}
}
