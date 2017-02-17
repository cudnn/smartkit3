package raft.jpct.bones.util;

import android.text.method.KeyListener;
import android.view.KeyEvent;
import android.view.MotionEvent;

import com.threed.jpct.Camera;
import com.threed.jpct.SimpleVector;

/** 
 * <p>Utility class to rotate {@link Camera} around a point with a fixed radius. 
 * Both orbit center and radius can be adjusted. This class can be added as a {@link KeyListener} to
 * {@link RenderPanel} to position camera according to key events.</p>
 * 
 * @author hakan eryargi (r a f t)
 * */
public class CameraOrbitController {
	
	public final SimpleVector cameraTarget = new SimpleVector(0, 0, 0);
	/** the angle with respect to positive Z axis. initial value is PI so looking down to positive Z axis. */
    public float cameraAngle = (float)(Math.PI);
	public float cameraRadius = 20f;
	public float cameraRotationSpeed = 0.1f;
	public float minCameraRadius = 3f;
	public float cameraMoveStepSize = 0.5f;
	
	public float dragTurnAnglePerPixel = (float) (Math.PI / 256);
	public float dragMovePerPixel = 1f;
	public float cameraMovePerWheelClick = 1.1f;
	
	private boolean cameraMovingUp = false;
	private boolean cameraMovingDown = false;
	private boolean cameraMovingIn = false;
	private boolean cameraMovingOut = false;
	private boolean cameraTurningLeft = false;
	private boolean cameraTurningRight = false;
	
 	private float dragStartX, dragStartY;
	private float cameraAngleAtDragStart = 0f;
	private float cameraHeightAtDragStart = 0f;
	
	private Camera camera;
	
	public CameraOrbitController(Camera camera) {
		this.camera = camera;
	}
	
	public boolean onKeyDown(int keyCode, KeyEvent event) {
		switch (keyCode) {
			case KeyEvent.KEYCODE_DPAD_UP:
				cameraMovingUp = true;
				return true;
			case KeyEvent.KEYCODE_DPAD_DOWN:
				cameraMovingDown = true;
				return true;
			case KeyEvent.KEYCODE_DPAD_RIGHT:
				cameraTurningRight = true;
				return true;
			case KeyEvent.KEYCODE_DPAD_LEFT:
				cameraTurningLeft = true;
				return true;
			case KeyEvent.KEYCODE_A:
				cameraMovingIn = true;
				return true;
			case KeyEvent.KEYCODE_Z:
				cameraMovingOut = true;
				return true;
		}
		return false;
	}

	public boolean onKeyUp(int keyCode, KeyEvent msg) {
		switch (keyCode) {
			case KeyEvent.KEYCODE_DPAD_UP:
				cameraMovingUp = false;
				return true;
			case KeyEvent.KEYCODE_DPAD_DOWN:
				cameraMovingDown = false;
				return true;
			case KeyEvent.KEYCODE_DPAD_RIGHT:
				cameraTurningRight = false;
				return true;
			case KeyEvent.KEYCODE_DPAD_LEFT:
				cameraTurningLeft = false;
				return true;
			case KeyEvent.KEYCODE_A:
				cameraMovingIn = false;
				return true;
			case KeyEvent.KEYCODE_Z:
				cameraMovingOut = false;
				return true;
		}
		return false;
	}
	
	public boolean onTouchEvent(MotionEvent event) {
		switch (event.getAction()) {
			case MotionEvent.ACTION_DOWN: 
		        dragStartX = event.getX();
		        dragStartY = event.getY();
		        cameraAngleAtDragStart = cameraAngle;
		        cameraHeightAtDragStart = cameraTarget.y;
		        return true;
			case MotionEvent.ACTION_MOVE: 
				cameraAngle = cameraAngleAtDragStart + (event.getX() - dragStartX) * dragTurnAnglePerPixel;
				cameraTarget.y = cameraHeightAtDragStart - (event.getY() - dragStartY) * dragMovePerPixel;
				return true;
		}
		
		return false;
	}
	
    public void placeCamera() {
    	if (cameraMovingUp)
			cameraTarget.y -= cameraMoveStepSize;
    	if (cameraMovingDown)
			cameraTarget.y += cameraMoveStepSize;
    	if (cameraMovingIn)
			cameraRadius = Math.max(cameraRadius - cameraMoveStepSize, minCameraRadius);
    	if (cameraMovingOut)
			cameraRadius += cameraMoveStepSize;
    	if (cameraTurningRight)
    		cameraAngle += cameraRotationSpeed;
    	if (cameraTurningLeft)
    		cameraAngle -= cameraRotationSpeed;
    	
        float camX = (float) Math.sin(cameraAngle) * cameraRadius;
        float camZ = (float) Math.cos(cameraAngle) * cameraRadius;
        
        SimpleVector camPos = new SimpleVector(camX, 0, camZ);
        camPos.add(cameraTarget);
        camera.setPosition(camPos);
        camera.lookAt(cameraTarget);
	}
	
    public void setPosition(SimpleVector translation){
    	camera.setPosition(translation);
    }
    
    public void setOrientation(SimpleVector dir , SimpleVector up){
    	camera.setOrientation(dir, up);
    	camera.setFOV(0.8816f);
    	camera.setYFOV(0.5173f);
    }

}
