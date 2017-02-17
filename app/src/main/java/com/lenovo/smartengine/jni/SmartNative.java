package com.lenovo.smartengine.jni;



public class SmartNative {
    /**
     * Set Screen Size Parameter
     * @param screenWidth
     * @param screenHeight
     * @return
     */
    public static native int setScreenSize(int screenWidth, int screenHeight);
    /**
     * Set Preivew Size Parameter
     * @param previewWidth
     * @param previewHeight
     * @return
     */
    public static native int setPreviewSize(int previewWidth, int previewHeight);
    /**
     * Scene Detection
     * @param data YUVBuffer
     * @return
     */
    public static native int sceneDetector(byte[] data);
    /**
     * Set Face Data
     * @param data
     * @param length
     * @return
     */
    public static native int setFaceData(int[] data, int length);
    //
    /**
     * Tuning Parameters
     * @param param1
     * @param len1
     * @param param2
     * @param len2
     * @return
     */
    public static native int setSmartParameter(int[] param1, int len1, float[] param2, int len2);
    
    /**
     * Set Trigger
     * @param value
     * @return
     */
    public static native int setTrigger(boolean value);
    
    /**
     * Set ISP Data
     * @param data
     * @param length
     * @return
     */
    public static native int setISPData(float[] data, int length);
    /**
     * Set Orientation Sensor Data
     * @param data
     * @param length
     * @return
     */
    public static native int setOsensorData(float[] data, int length);
    /**
     * Set Accelerator Data
     * @param data
     * @param length
     * @param ori  Screen Orientation(0~359)
     * @return
     */
    public static native int setAsensorData(float[] data, int length);
    /**
     * Set Front Light Sensor Data
     * @param lsensor
     * @return
     */
    public static native int setLsensorData(float[] data, int length);
    /**
     * set the orientation of camera
     * @param orientation
     * @return
     */
    public static native int setOrientation(int orientation);
    
    /**
     * Set ROI Data
     * @param data
     * @param length
     * @return
     */
    public static native int setROIData(int[] data, int length);
    
    /**
     * Get CompositeResult (from Hal to App)
     * 0: scene_flag
     * 1: scene_guide_allo
     * 2: ratio_angle
     * 3: zoom_flag
     * 4: current_position_x
     * 5: current_position_y
     * 6: screen_orientation
     * 7: scene_guide_flag
     * 8: target_transparency_left
     * 9: target_transparency_right
     * 10:is_height_ok
     * 11:is_angle_ok
     * @return
     */
    
    
    public static native int[] getCompositeResult();
    /**
     * Get Result of Scene Detection( 0: Normal Scene; 1: Motion Scene)
     * @return
     */
    public static native int[] getSceneResult();
    /**
     * Get QRCode position
     * @return
     */
    public static native float[] getQRPostion();
    /**
     * Get QR Code decode describer
     * @return
     */
    public static native String getQRCodeDecodeTxt();
    /**
     * Get QR Code decode describer
     * @return
     */
    public static native String getQRCodeDecodeType();
    /**
     * Get food attractiveness score
     * @return
     */
    public static native float getFoodAttractiveness();
    /**
     * Get variable for debugging
     * @return
     */
    public static native float[] getDebugValue();
    // hehe from xufeng 20160302 10:32
    public static native float[] getMVPMatrix();
    public static native float[] getExtrinsicMatrix();
    public static native float[] getGlProjectionMatrix();
    //Load smart native .so file 11:19
    static {
        System.loadLibrary("smart_kit");
        System.loadLibrary("smart_kit_jni");
    }
}
