/*
 * smart_extern_manager.cpp
 *
 *  Created on: 2015/07/28
 *      Author:
 *
 * 	Add comments on variable and functions
 *
 *  Revised on: 2016/04/7
 *      Author: fengbin1
 */

#define _ANDROID_
#define DEBUG

#include "smart_extern_manager.h"
#include <stdio.h>
#include "smart_type.h"
#include "log.h"

namespace smart {

/**
 * Constructs a new instance of SmartExternManager
 */
SmartExternManager::SmartExternManager() {
	sensor_enable_flag = 0;

	for (int i = 0; i < 21; i++) {
		face_data[i] = 0;
	}
	for (int i = 0; i < 13; i++) {
		isp_data[i] = 0;
	}
	for (int i = 0; i < 3; i++) {
		a_sensor[i] = 0;
	}
	for (int i = 0; i < 3; i++) {
		o_sensor[i] = 0;
	}
	for (int i = 0; i < 3; i++) {
		l_sensor[i] = 0;
	}
	for (int i = 0; i < 5; i++) {
		roi_data[i] = 0;
	}


	is_isp_usable = false;
	
	orientation_raw = 0;
	orientation = 0;

	preview_width = 0;
	preview_height = 0;
	screen_width = 0;
	screen_height = 0;

	is_roi_selected = false;
	trigger_value = false;
}
/**
 * SmartExternManager destructors
 */
SmartExternManager::~SmartExternManager() {
	// TODO Auto-generated destructor stub
}
/**
 * Tells which sensor is support
 * @param bit    sensor type (SMART_SENSOR_TYPE_ACCELEROMETER|SMART_SENSOR_TYPE_ORIENTATION|SMART_SENSOR_TYPE_LIGHT)
 * @return
 */
bool SmartExternManager::enable_sensor(const int bit) {
	sensor_enable_flag |= bit;
	return true;
}
/**
 * Tells which sensor is not support
 * @param bit    sensor type (SMART_SENSOR_TYPE_ACCELEROMETER|SMART_SENSOR_TYPE_ORIENTATION|SMART_SENSOR_TYPE_LIGHT)
 * @return
 */
bool SmartExternManager::disable_sensor(const int bit) {
	sensor_enable_flag &= ~bit;
	return true;
}
/**
 * judge which sensor is enabled
 * @param bit    sensor type (SMART_SENSOR_TYPE_ACCELEROMETER|SMART_SENSOR_TYPE_ORIENTATION|SMART_SENSOR_TYPE_LIGHT)
 * @return
 */
bool SmartExternManager::is_enable(const int bit) {
	return (sensor_enable_flag & bit) == bit;
}

bool SmartExternManager::set_trigger(const bool trigger) {

	trigger_value = trigger ;
	return true;
}

/**
 * Updates when sensor values have changed
 * @param type    sensor type (SMART_SENSOR_TYPE_ACCELEROMETER|SMART_SENSOR_TYPE_ORIENTATION|SMART_SENSOR_TYPE_LIGHT)
 * @param data    sensor value(forces or rotation of x y z, meybe the lux)
 * @param length  the data length
 */
bool SmartExternManager::update_sensor(const int type, float* data, const int length) {
	float* dest;
	switch (type) {
	case SMART_SENSOR_TYPE_ACCELEROMETER:
		dest = a_sensor;
		break;
	case SMART_SENSOR_TYPE_ORIENTATION:
		dest = o_sensor;
		break;
	case SMART_SENSOR_TYPE_LIGHT:
		dest = l_sensor;
		break;
	default:
		return false;
	}
	for (int i = 0; i < length; i++) {
		dest[length+i] = dest[i];
	}
	for (int i = 0; i < length; i++) {
		dest[i] = data[i];
	}

//	if (type == SMART_SENSOR_TYPE_ACCELEROMETER) {
//		SMART_DEBUG("update_sensor a =%f,%f,%f,%f,%f,%f", dest[0], dest[1], dest[2], dest[3], dest[4], dest[5]);
//	}
//	if (type == SMART_SENSOR_TYPE_ORIENTATION) {
//		SMART_DEBUG("update_sensor o =%f,%f,%f,%f,%f,%f", dest[0], dest[1], dest[2], dest[3], dest[4], dest[5]);
//	}
//	if (type == SMART_SENSOR_TYPE_LIGHT) {
//		SMART_DEBUG("update_sensor l =%f,%f", dest[0], dest[1]);
//	}
	return true;
}
/**
 * Updates the detected faces in the preview frame.
 * @param data    the detected faces pos [left,top,right,bottom,left2,top2...]
 * @param length  faces num
 */
bool SmartExternManager::update_face(const int* data, const int length) {
	face_data[0] = length;
	float scale_x = preview_width / 2000.f;//0.96
	float scale_y = preview_height / 2000.f;//0.54
	for (int i = 0; i < length; i++) {
		face_data[i * 4 + 1] = (data[i * 4 + 0] + 1000) * scale_x;  //left
		face_data[i * 4 + 2] = (data[i * 4 + 1] + 1000) * scale_y;  //top
		face_data[i * 4 + 3] = (data[i * 4 + 2] + 1000) * scale_x;  //right
		face_data[i * 4 + 4] = (data[i * 4 + 3] + 1000) * scale_y;  //bottom
	}
	SMART_DEBUG("update_face =%d,%d,%d,%d,%d", face_data[0], face_data[1], face_data[2], face_data[3], face_data[4]);
	return true;
}
/**
 * Updates the isp data when the device has changed
 * @param data    0:status; 1:aec-gain; 2:linecount; 3:exp_index; 4:exp_time; 5:lux_index; 6:CCT;
 * 			      7:R_gain; 8:G_gain; 9:B_gain; 10:awb-decision; 11:focus_pos; 12:focus_dis;
 * @param length 13
 */
bool SmartExternManager::update_isp(const float* data, const int length) {
	for (int i = 0; i < length; i++) {
		isp_data[i] = data[i];
	}
	if (length > 0) {
		is_isp_usable = true;
	} else {
		is_isp_usable = false;
	}
	return true;
}
/**
 * Updates the orientation when the device has changed
 * @param ori    orientation parameter is in degrees, ranging from 0 to 359
 */
bool SmartExternManager::update_orientation(const int ori) {
	orientation_raw = ori;
	orientation = ((ori + 45) / 90 * 90) % 360;
	SMART_DEBUG("%s orientation: %d - %d", __func__, orientation_raw, orientation);
	return true;
}
/**
 * Sets the dimensions for preview pictures.
 * @param width    the width of the pictures, in pixels
 * @param height   the height of the pictures, in pixels
 */
bool SmartExternManager::set_preview_size(const int width, const int height) {
	SMART_DEBUG("%s preivew size %dx%d", __func__, width, height);
	__android_log_print(ANDROID_LOG_INFO, "SMART_PLANET", "exManager: set_preview_size - E.  w*h=[ %d, %d ]" , preview_width ,  preview_height  );
	preview_width = width;
	preview_height = height;
	__android_log_print(ANDROID_LOG_INFO, "SMART_PLANET", "exManager: set_preview_size - X  w*h=[ %d, %d ]" , preview_width ,  preview_height  );
	return true;
}
/**
 * Sets the dimensions for screen size
 * @param width    the absolute width of the display in pixels.
 * @param height   the absolute height of the display in pixels.
 */
bool SmartExternManager::set_screen_size(const int width, const int height) {
	SMART_DEBUG("%s screen size %dx%d", __func__, width, height);
	screen_width = width;
	screen_height = height;
	__android_log_print(ANDROID_LOG_INFO, "SMART_PLANET", "exManager: set_screen_size - X w*h=[ %d, %d ]" , screen_width ,  screen_height  );
	return true;
}

/**
* Updates the region of interest in the preview frame.
* @param data    the detected roi pos [left,top,right,bottom,left2,top2...]
* @param length  roi num
*/
bool SmartExternManager::update_roi(const int* data, const int length) {

	__android_log_print(ANDROID_LOG_INFO, "SMART_PLANET", "exManager: screen_size  w*h=[ %d, %d ]" , screen_width ,  screen_height  );
	__android_log_print(ANDROID_LOG_INFO, "SMART_PLANET", "exManager: preview_size w*h=[ %d, %d ]" , preview_width,  preview_height );

	/*
	 * using for deal with the screen w & h unsync
	 */

	int screen_width_for_ratio ;

	if( screen_height > screen_width)
		screen_width_for_ratio = screen_height;
	else
		screen_width_for_ratio = screen_width ;

	float ratio = preview_width / (float)screen_width_for_ratio ;

	//float ratio = preview_width / (float)screen_height ;

	__android_log_print(ANDROID_LOG_INFO, "SMART_PLANET", "exManager: ratio = %f", ratio );

	roi_data[0] = length;
	//float scale_x = preview_width / 2000.f;//0.96
	//float scale_y = preview_height / 2000.f;//0.54
	for (int i = 0; i < length; i++) {
		roi_data[i * 4 + 1] = data[i * 4 + 1]*ratio;
		roi_data[i * 4 + 2] = data[i * 4 + 2]*ratio;
		roi_data[i * 4 + 3] = data[i * 4 + 3]*ratio;
		roi_data[i * 4 + 4] = data[i * 4 + 4]*ratio;
		//face_data[i * 4 + 1] = (data[i * 4 + 0] + 1000) * scale_x;  // left
		//face_data[i * 4 + 2] = (data[i * 4 + 1] + 1000) * scale_y;  //  top
		//face_data[i * 4 + 3] = (data[i * 4 + 2] + 1000) * scale_x;  // right
		//face_data[i * 4 + 4] = (data[i * 4 + 3] + 1000) * scale_y;  //bottom
	}

	int dis     = ( roi_data[3] - roi_data[1] ) * ( roi_data[3] - roi_data[1] ) + ( roi_data[4] - roi_data[2] ) * ( roi_data[4] - roi_data[2] ) ;
	int dis_thd_1 = ( ( preview_width * preview_width ) / 100 ) ;
	int dis_thd_2 = ( ( preview_width * preview_width ) / 5 ) ;

	if( dis < dis_thd_1 ||  dis > dis_thd_2 ){
		is_roi_selected = false;

		__android_log_print(ANDROID_LOG_INFO, "SMART_PLANET", "exManager: update_roi=out of range  dis=%d , dis_thd_1 = %d , dis_thd_2 = %d " , dis,  dis_thd_1 , dis_thd_2 );

		__android_log_print(ANDROID_LOG_INFO, "SMART_PLANET", "exManager: update_roi_screen  =%d , [ %d , %d , %d , %d ] , w=%d , h=%d",     data[0],     data[1],     data[2],     data[3],     data[4],screen_width ,screen_height );
		__android_log_print(ANDROID_LOG_INFO, "SMART_PLANET", "exManager: update_roi_preview =%d , [ %d , %d , %d , %d ] , w=%d , h=%d", roi_data[0], roi_data[1], roi_data[2], roi_data[3], roi_data[4],preview_width , preview_height );

	}else{
		is_roi_selected = true;
		__android_log_print(ANDROID_LOG_INFO, "SMART_PLANET", "exManager: update_roi=true dis=%d , dis_thd_1 = %d , dis_thd_2 = %d " , dis,  dis_thd_1 , dis_thd_2 );

		__android_log_print(ANDROID_LOG_INFO, "SMART_PLANET", "exManager: update_roi_screen  =%d , [ %d , %d , %d , %d ] , w=%d , h=%d",     data[0],     data[1],     data[2],     data[3],     data[4],screen_width ,screen_height );
		__android_log_print(ANDROID_LOG_INFO, "SMART_PLANET", "exManager: update_roi_preview =%d , [ %d , %d , %d , %d ] , w=%d , h=%d", roi_data[0], roi_data[1], roi_data[2], roi_data[3], roi_data[4],preview_width , preview_height );
	}

	return true;
}

SmartExternManager* SmartExternManager::instance_ = new SmartExternManager();
/**
 * Returns the global SmartExternManager instance
 */
SmartExternManager* SmartExternManager::get_instance() {
	return instance_;
}
}/* namespace smart */
