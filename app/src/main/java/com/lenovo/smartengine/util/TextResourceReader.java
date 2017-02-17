package com.lenovo.smartengine.util;

import java.io.BufferedReader;
import java.io.InputStream;
import java.io.InputStreamReader;

import android.content.Context;

public class TextResourceReader {
	
	public static String readTextFileFromResource( Context context , int resourceId  ){
		StringBuilder body = new StringBuilder();
		
		try {
			InputStream inputStream = context.getResources().openRawResource(resourceId);
			InputStreamReader inputStreamReader = new InputStreamReader(inputStream);
			BufferedReader bufferReader = new BufferedReader(inputStreamReader);
			String nextLine;
			
			while ( ( nextLine = bufferReader.readLine()) != null) {
				body.append(nextLine);
				body.append('\n');
			}
		} catch (Exception e) {
			// TODO: handle exception
			throw new RuntimeException("Could not open resource:"+resourceId,e);
		} 
		
		return body.toString();
		
	}

}
