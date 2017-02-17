/***
 * Excerpted from "OpenGL ES for Android",
 * published by The Pragmatic Bookshelf.
 * Copyrights apply to this code. It may not be used to create training material, 
 * courses, books, articles, and the like. Contact us if you are in doubt.
 * We make no guarantees that this code is fit for any purpose. 
 * Visit http://www.pragmaticprogrammer.com/titles/kbogla for more book information.
***/
package com.lenovo.smartengine.util;

public class MatrixHelper {
    public static void perspectiveM(float[] m, float yFovInDegrees, float aspect,
        float n, float f) {
        final float angleInRadians = (float) (yFovInDegrees * Math.PI / 180.0);
        final float a = (float) (1.0 / Math.tan(angleInRadians / 2.0));

        m[0] = a / aspect;
        m[1] = 0f;
        m[2] = 0f;
        m[3] = 0f;

        m[4] = 0f;
        m[5] = a;
        m[6] = 0f;
        m[7] = 0f;

        m[8] = 0f;
        m[9] = 0f;
        m[10] = -((f + n) / (f - n));
        m[11] = -1f;
        
        m[12] = 0f;
        m[13] = 0f;
        m[14] = -((2f * f * n) / (f - n));
        m[15] = 0f;        
    }
    
    public static void replaceM(float[] m) {
		m[0]= 167.17f ;
		m[1]= -12.21f; 
		m[2]= -13.12f ;
		m[3]= -13.07f ;
		m[4]=  28.18f;
		m[5]= 115.49f;
		m[6]=  66.23f;
		m[7]=  65.96f;
		m[8]=  -6.82f;
		m[9]=  178.30f;
		m[10]= -43.51f;
		m[11]= -43.51f;
		m[12]= -261.11f;
		m[13]= 133.20f;
		m[14]= 409.28f;
		m[15]= 427.61f;       
    }
}
