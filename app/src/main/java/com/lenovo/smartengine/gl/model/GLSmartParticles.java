package com.lenovo.smartengine.gl.model;

import android.opengl.GLES20;
import static android.opengl.Matrix.multiplyMM;
import static android.opengl.Matrix.setIdentityM;
import static android.opengl.Matrix.translateM;

import java.util.Random;

import com.particles.android.objects.ParticleFireworksExplosion;
import com.particles.android.objects.ParticleShooter;
import com.particles.android.objects.ParticleSystem;
import com.particles.android.programs.ParticleShaderProgram;
import com.particles.android.util.MatrixHelper;
import com.particles.android.util.Geometry.Point;
import com.particles.android.util.Geometry.Vector;

import android.content.Context;
import android.graphics.Color;

public class GLSmartParticles {
	
	private final Context context;
	
    private final float[] projectionMatrix = new float[16];    
    private final float[] viewMatrix = new float[16];
    private final float[] viewProjectionMatrix = new float[16];	    
    private ParticleShaderProgram particleProgram;      
    private ParticleSystem particleSystem;
    private ParticleShooter redParticleShooter;
    private ParticleShooter greenParticleShooter;
    private ParticleShooter blueParticleShooter;
    
//    private ParticleFireworksExplosion particleFireworksExplosion;
    private Random random;
    private final float[] hsv = {0f, 1f, 1f};
    
    private long globalStartTime;
	
	public GLSmartParticles(Context context){
		this.context = context;
	}
	
	public void GLSmartParticlesOnSurfaceCreated(){

        
        particleProgram = new ParticleShaderProgram(context);        
        particleSystem = new ParticleSystem(10000);        
        globalStartTime = System.nanoTime();
        
        final Vector particleDirection = new Vector(0.0f, 0.0f, 0.5f);
        
        final float angleVarianceInDegrees = 30f; 
        final float speedVariance = 1f;
        
        redParticleShooter = new ParticleShooter(
                new Point(-0.3f, 0f, -0.5f), 
                particleDirection,                
                Color.rgb(255, 50, 5),            
                angleVarianceInDegrees, 
                speedVariance);
            
            greenParticleShooter = new ParticleShooter(
                new Point(0f, 0f, -0.5f), 
                particleDirection,
                /*Color.rgb(25, 255, 25)*/Color.rgb(255, 50, 5),            
                angleVarianceInDegrees, 
                speedVariance);
            
            blueParticleShooter = new ParticleShooter(
                new Point(0.3f, 0f, -0.5f), 
                particleDirection,
                /*Color.rgb(5, 50, 255)*/Color.rgb(255, 50, 5),            
                angleVarianceInDegrees, 
                speedVariance); 
        
//            particleFireworksExplosion = new ParticleFireworksExplosion();
            
            random = new Random(); 
	}
	
	public void GLSmartParticlesOnSurfaceChanged( int width, int height){
		
		
		//------------------------------------------------------------------
        MatrixHelper.perspectiveM(projectionMatrix, 45, (float) width / (float) height, 1f, 10f);
            
        setIdentityM(viewMatrix, 0);
        translateM(viewMatrix, 0, 0f, -1.5f, -5f);   
        multiplyMM(viewProjectionMatrix, 0, projectionMatrix, 0,viewMatrix, 0);
		//------------------------------------------------------------------
	}
	
	public void GLSmartParticlesOnDramFrame(float[] projectMatrix){
		//-------------------------------------------------------------------
		GLES20.glEnable(GLES20.GL_BLEND);
		GLES20.glBlendFunc(GLES20.GL_ONE,GLES20.GL_ONE);
		
	       float[] temo = projectMatrix ;//ARNativeActivity.getProjectMatrix();
	        if(temo[0]!= 0 && temo[1] != 0  ){
	        	
	        	if(particleSystem.reset == false){
	        		particleSystem.resetParticle();
	        		particleSystem.reset = true;
	        		globalStartTime = System.nanoTime();
	        	}
	        	
	            float currentTime = (System.nanoTime() - globalStartTime) / 1000000000f;

	            redParticleShooter.addParticles  (particleSystem, currentTime, 1);
	            greenParticleShooter.addParticles(particleSystem, currentTime, 3);              
	            blueParticleShooter.addParticles (particleSystem, currentTime, 5);
	            
	            
/*	            if (random.nextFloat() < 0.02f) {
	                hsv[0] = random.nextInt(360);
	                
	                particleFireworksExplosion.addExplosion(
	                particleSystem, 
	                new Point(
	                    -1f + random.nextFloat() * 2f,
	                    1f  + random.nextFloat() / 2f, 
	                    3f  + random.nextFloat() * 2f),
	                Color.HSVToColor(hsv), 
	                globalStartTime);                              
	                
	            }  */
	            
	            particleProgram.useProgram();
	            
	            //particleProgram.setUniforms(viewProjectionMatrix, currentTime, texture);           
	            particleProgram.setUniforms(/*ARNativeActivity.getProjectMatrix()*/projectMatrix, currentTime/*, texture*/);
	            
	            particleSystem.bindData(particleProgram);
	            particleSystem.draw(); 
	        }else{
	        	particleSystem.reset = false;
	        }
	        GLES20.glDisable(GLES20.GL_BLEND);
			//-------------------------------------------------------------------
	}
	
}
