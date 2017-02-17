package com.lenovo.smartengine.gl.model;

import java.util.LinkedList;
import java.util.List;

import raft.jpct.bones.Animated3D;
import raft.jpct.bones.AnimatedGroup;
import raft.jpct.bones.BonesIO;
import raft.jpct.bones.util.CameraOrbitController;
import android.content.Context;
import android.content.res.Resources;

import com.threed.jpct.Camera;
import com.threed.jpct.FrameBuffer;
import com.threed.jpct.Interact2D;
import com.threed.jpct.Light;
import com.threed.jpct.Logger;
import com.threed.jpct.Matrix;
import com.threed.jpct.Object3D;
import com.threed.jpct.RGBColor;
import com.threed.jpct.SimpleVector;
import com.threed.jpct.Texture;
import com.threed.jpct.TextureManager;
import com.threed.jpct.World;

public class GLSmartPlugin {


	private final Context context;

	private World world = null;
	private AnimatedGroup masterModel;
	private CameraOrbitController cameraController;
	private final List<AnimatedGroup> models = new LinkedList<AnimatedGroup>();

	private FrameBuffer frameBuffer = null;

	private long aggregatedTime = 0;
	private long frameTime = System.currentTimeMillis();
	private static final int GRANULARITY = 25;

	public static  int animation;
	private float animateSeconds = 0f;
	private float speed = 1f;
	
	private boolean pickingEnable ;
	public  static boolean  actionEventDownTure ;
	public  static float    actionEventDown_x ;
	public  static float    actionEventDown_y ;
	public static long timestart ;
	public static boolean isstart = false;
	private long timecurrent;
	
	public GLSmartPlugin(Context context){
		this.context = context;
		actionEventDownTure  = false;
		actionEventDown_x = -1;
		actionEventDown_y = -1;
		timestart = 0;
		timecurrent = 0;
		tx = 0;
		
		pickingEnable = false;
		animation = 1;
	}
	
	public void SmartGLRenderPligginPicking( boolean enable ){
		pickingEnable = enable;
	}
	
	public void SmartGLRenderPlugginOnSurfaceCreated(int modelID , int textureID){
		if (world != null)
			return;

		world = new World();

		try {
			Resources res = context.getResources();
			masterModel = BonesIO.loadGroup(res.openRawResource(modelID));
			addModel();
		} catch (Exception e) {
			e.printStackTrace();
			throw new RuntimeException(e);
		}

		world.setAmbientLight(227, 227, 227);
		world.buildAllObjects();

		float[] bb = calcBoundingBox();
		float height = (bb[3] - bb[2]); // ninja height
		//new Light(world).setPosition(new SimpleVector(0, -height / 2, height));

		cameraController = new CameraOrbitController(world.getCamera());
		cameraController.cameraAngle = 0;	

		TextureManager.getInstance().flush();
		Resources res = context.getResources();

		Texture texture = new Texture(res.openRawResource(textureID));
		texture.keepPixelData(true);
		TextureManager.getInstance().addTexture("modelTexture", texture);

		for (AnimatedGroup group : models) {
			for (Animated3D a : group){
				a.setTexture("modelTexture");
				if(pickingEnable == true)
					a.setCollisionMode(Object3D.COLLISION_CHECK_OTHERS);
			}
		}
	}
	
	public void SmartGLRenderPlugginOnSurfaceChanged( int width , int height ){
		if (frameBuffer != null) {
			frameBuffer.dispose();
		}
		frameBuffer = new FrameBuffer(/*gl,*/ width, height);
			
		autoAdjustCamera();
	}
	

	
	public void SmartGLRenderPlugginOnDramFrame( float[] projectMatrixint , float[] transformationM ){
		if (frameBuffer == null)
			return;

		if(isstart == true){
			timestart = System.currentTimeMillis();
			animation = 1;
			isstart = false;
		}
		

		
		long now = System.currentTimeMillis();
		aggregatedTime += (now - frameTime);
		frameTime = now;

		if (aggregatedTime > 1000) {
			aggregatedTime = 0;
		}

		while (aggregatedTime > GRANULARITY) {
			aggregatedTime -= GRANULARITY;
			animateSeconds += GRANULARITY * 0.001f * speed;
			cameraController.placeCamera();
		}

		Matrix projMatrix = new Matrix();
		projMatrix.setDump(projectMatrixint);
		projMatrix.transformToGL();
		SimpleVector translation = projMatrix.getTranslation();
		SimpleVector dir = projMatrix.getZAxis();
		SimpleVector up = projMatrix.getYAxis();
		cameraController.setPosition(translation);
		cameraController.setOrientation(dir, up);

		
		if ( pickingEnable == true && actionEventDownTure == true) {
				
			SimpleVector dir2= Interact2D.reproject2D3DWS(world.getCamera(), frameBuffer, (int)actionEventDown_x, (int)actionEventDown_y).normalize();
			Object[] res    = world.calcMinDistanceAndObject3D(world.getCamera().getPosition(), dir2, 10000 /*or whatever*/);
			
			actionEventDown_x = -1;
			actionEventDown_y = -1;
			actionEventDownTure = false;
			
			if(res[1] != null){			
				
				animation = 2;
				timestart = System.currentTimeMillis();
			}
		}
		
		if ( animation == 2){
			timecurrent = System.currentTimeMillis();
			float cycle = ( timecurrent - timestart ) /1000.f ;
			if( cycle >=  masterModel.getSkinClipSequence().getClip(animation - 1).getTime()){
				animation = 1 ;
			}else{
				animation = 2 ;
			}
		}
		
		if (animation > 0 && masterModel.getSkinClipSequence().getSize() >= animation) {
			float clipTime = masterModel.getSkinClipSequence().getClip(animation - 1).getTime();
			if (animateSeconds > clipTime) {
				animateSeconds = 0;
			}
			float index = animateSeconds / clipTime;

			for (AnimatedGroup group : models) {
				group.animateSkin(index, animation);
				
				for (Animated3D a : group) {

					// a.animate(index, animation);

					Matrix dump = new Matrix();
					dump.setDump(transformationM);
					dump.transformToGL();

					a.clearTranslation();
					a.translate(dump.getTranslation());

					//dump.setRow(3,0.0f,0.0f,0.0f,1.0f);
					a.clearRotation();
					a.setRotationMatrix(dump);
					a.scale(0.1f);
				}
			}

		} else {
			animateSeconds = 0f;
		}
		

		
		// frameBuffer.clear();
		world.renderScene(frameBuffer);
		world.draw(frameBuffer);
		//world.drawWireframe(frameBuffer, RGBColor.RED, 3, false);//world.draw(frameBuffer);
		frameBuffer.display();

	}
	
	float tx = 0;

	
	private void addModel() {

		AnimatedGroup model = masterModel.clone(AnimatedGroup.MESH_DONT_REUSE);
		model.addToWorld(world);
		models.add(model);
		Logger.log("added new model: " + models.size());
	}
	
	private float[] calcBoundingBox() {
		float[] box = null;

		for (Animated3D skin : masterModel) {
			float[] skinBB = skin.getMesh().getBoundingBox();

			if (box == null) {
				box = skinBB;
			} else {
				// x
				box[0] = Math.min(box[0], skinBB[0]);
				box[1] = Math.max(box[1], skinBB[1]);
				// y
				box[2] = Math.min(box[2], skinBB[2]);
				box[3] = Math.max(box[3], skinBB[3]);
				// z
				box[4] = Math.min(box[4], skinBB[4]);
				box[5] = Math.max(box[5], skinBB[5]);
			}
		}
		return box;
	}
	
	private void autoAdjustCamera() {
		float[] bb = calcBoundingBox();
		float groupHeight = bb[3] - bb[2];
		cameraController.cameraRadius = calcDistance(world.getCamera(),frameBuffer, frameBuffer.getHeight() / 1.5f, groupHeight);
		cameraController.minCameraRadius = groupHeight / 10f;
		cameraController.cameraTarget.y = (bb[3] + bb[2]) / 2;
		cameraController.placeCamera();
	}
	
	private float calcDistance(Camera c, FrameBuffer buffer, float height,float objectHeight) {
		float h = height / 2f;
		float os = objectHeight / 2f;

		Camera cam = new Camera();
		cam.setFOV(c.getFOV());
		SimpleVector p1 = Interact2D.project3D2D(cam, buffer, new SimpleVector(0f, os, 1f));
		float y1 = p1.y - buffer.getCenterY();
		float z = (1f / h) * y1;

		return z;
	}
}
