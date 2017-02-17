/*
 * smart_extern_manager.h
 *
 *  Created on: 2015年7月28日
 *      Author: guoyj7
 */

#ifndef SMART_KIT_SMART_EXTERN_MANAGER_H_
#define SMART_KIT_SMART_EXTERN_MANAGER_H_

namespace smart {

class SmartExternManager {
public:
	/**
	 * Tells which sensor is support
	 * @param bit    sensor type (SMART_SENSOR_TYPE_ACCELEROMETER|SMART_SENSOR_TYPE_ORIENTATION|SMART_SENSOR_TYPE_LIGHT)
	 * @return
	 */
	bool enable_sensor(const int bit);
	/**
	 * Tells which sensor is not support
	 * @param bit    sensor type (SMART_SENSOR_TYPE_ACCELEROMETER|SMART_SENSOR_TYPE_ORIENTATION|SMART_SENSOR_TYPE_LIGHT)
	 * @return
	 */
	bool disable_sensor(const int bit);
	/**
	 * judge which sensor is enabled
	 * @param bit    sensor type (SMART_SENSOR_TYPE_ACCELEROMETER|SMART_SENSOR_TYPE_ORIENTATION|SMART_SENSOR_TYPE_LIGHT)
	 * @return
	 */
	bool is_enable(const int bit);

	bool set_trigger(const bool trigger);
	/**
	 * Updates when sensor values have changed
	 * @param type    sensor type (SMART_SENSOR_TYPE_ACCELEROMETER|SMART_SENSOR_TYPE_ORIENTATION|SMART_SENSOR_TYPE_LIGHT)
	 * @param data    sensor value(forces or rotation of x y z, meybe the lux)
	 * @param length  the data length
	 */
	bool update_sensor(const int type, float* data, const int length);
	/**
	 * Updates the detected faces in the preview frame.
	 * @param data    the detected faces pos [left,top,right,bottom,left2,top2...]
	 * @param length  faces num
	 */
	bool update_face(const int* data, const int length);
	/**
	 * Updates the isp data when the device has changed
	 * @param data    0:status; 1:aec-gain; 2:linecount; 3:exp_index; 4:exp_time; 5:lux_index; 6:CCT;
	 * 			      7:R_gain; 8:G_gain; 9:B_gain; 10:awb-decision; 11:focus_pos; 12:focus_dis;
	 * @param length 13
	 */
	bool update_isp(const float* data, const int length);
	/**
	 * Updates the orientation when the device has changed
	 * @param ori    orientation parameter is in degrees, ranging from 0 to 359
	 */
	bool update_orientation(const int orientation);
	/**
	 * Sets the dimensions for preview pictures.
	 * @param width    the width of the pictures, in pixels
	 * @param height   the height of the pictures, in pixels
	 */
	bool set_preview_size(const int width, const int height);
	/**
	 * Sets the dimensions for screen size
	 * @param width    the absolute width of the display in pixels.
	 * @param height   the absolute height of the display in pixels.
	 */
	bool set_screen_size(const int width, const int height);

	/**
	* Updates the region of interest in the preview frame.
	* @param data    the detected roi pos [left,top,right,bottom,left2,top2...]
	* @param length  roi num
	*/
	bool update_roi(const int* data, const int length);

	static SmartExternManager* get_instance();

	int face_data[21];
	float isp_data[13];
	float a_sensor[6];
	float o_sensor[6];
	float l_sensor[6];

	bool is_isp_usable;

	int orientation_raw;
	int orientation;

	int preview_width;
	int preview_height;
	int screen_width;
	int screen_height;

	bool is_roi_selected;
	int  roi_data[5];

	bool trigger_value ;

private:
	SmartExternManager();
	virtual ~SmartExternManager();
	static SmartExternManager* instance_;

	int sensor_enable_flag;
};

} /* namespace smart */
#endif /* SMART_KIT_SMART_EXTERN_MANAGER_H_ */
