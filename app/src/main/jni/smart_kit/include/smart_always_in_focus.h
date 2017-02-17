/*
 * smart_always_in_focus.h
 *
 *  Created on: 2016/07/11
 *      Author: xufeng8
 *
 * 	Add comments on variable and functions, clear up the unused part
 *
 *  Revised on:
 *      Author:
 */

#include "data_structs.h"
#include "smart_extern_manager.h"


namespace smart {

//class MFTracking;
class TLD;
class SmartAlwaysInFocus {
	private:
		smart::SmartExternManager* extern_manager;      //sensor data manager

		Image* input_frame;                             //original data, for always in foucs
		Image* resize_frame;                            //data for tracking

		int process_image_width;             			//width of processed image for recognition and tracking, may be used in composition guide
		int process_image_height;		     			//height of processed image for recognition and tracking, may be used in composition guide

		static const int DownsampleWidth16_9  = 320;    //720 , 480 size of code(16:9)
		static const int DownsampleHeight16_9 = 180;	//405 , 270
		static const int DownsampleWidth4_3   = 320;	//640;      //size of code(4:3)
		static const int DownsampleHeight4_3  = 240;	//480;
		int track_frame;                                //number of tracking frame

		TLD* tld_tracker;

		bool is_tracked;
		bool is_tracking;


		bool preview_ratio_check(const int width, const int height);

	public:

		SmartAlwaysInFocus();
		virtual ~SmartAlwaysInFocus();

		bool get_tracking_status();
		bool get_tracking_result();
		bool get_tracking_stop();
		void reset_tracking_stop();
		void isable_tracking_stop();
		void init_clear();

		int* rect_for_chaser ;	 						//position for composition guide: left top right bottom center_x center_y

		void process_frame(unsigned char* input, int width, int height);
		void init(unsigned char* input, int width, int height);
		void do_tracking(unsigned char* input, int width, int height);
		void status_reset();
		bool tracking_stop;
		int  pEx_length ;
	};
}
