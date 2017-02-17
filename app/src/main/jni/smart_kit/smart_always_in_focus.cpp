#include "smart_always_in_focus.h"
//#include "mf_tracking.h"
#include "util.h"

#include "./obj_tracking/tld_tracking/TLD.h"

#include <android/log.h>

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include <time.h>

smart::SmartAlwaysInFocus::SmartAlwaysInFocus(){

	extern_manager = smart::SmartExternManager::get_instance();

	resize_frame   = new Image(DownsampleWidth16_9, DownsampleHeight16_9, 320 * 240) ;//720x405

	is_tracking = false;
	is_tracked  = false;
	tracking_stop = false;
	track_frame = 0;
	pEx_length = 0;
	rect_for_chaser   = new int[4] ;   //memset(rect_for_chaser, 0, sizeof(rect_for_chaser));

	tld_tracker   = new TLD() ;


}

smart::SmartAlwaysInFocus::~SmartAlwaysInFocus(){

	delete[] rect_for_chaser;
	delete resize_frame;

	tld_tracker = NULL;
	delete tld_tracker;

}


//void smart::SmartAlwaysInFocus::reset(){
//	tld_tracker->init_clear();
//}

void smart::SmartAlwaysInFocus::process_frame(unsigned char* input, int width, int height){

//	long beginTime = clock();

//	if( extern_manager->trigger_value == true ){
//		tld_tracker->init_clear();
//		status_reset();
//		extern_manager->trigger_value = false;
//
//		tracking_stop = true;
//		__android_log_print(ANDROID_LOG_INFO, "SMART_CHASER_TLD", "process_frame : tracking_stop , frame = %d" ,track_frame );
//		return ;
//	}

	if(is_tracking == false){
		memset (rect_for_chaser,-1,4);
		is_tracking = true;
		init(input , width , height);
		__android_log_print(ANDROID_LOG_INFO, "SMART_PLANET", "layer2: process_frame : initialization , frame = %d" ,track_frame );
	}else{
		do_tracking(input , width , height);
		__android_log_print(ANDROID_LOG_INFO, "SMART_PLANET", "layer2: process_frame : processing       frame = %d" ,track_frame );
	}

//	long endTime = clock() - beginTime;

//	__android_log_print(ANDROID_LOG_INFO, "SMART_CHASER_TLD", "process_frame : process time = %d ms" ,(int)( endTime/1000 ) );


	track_frame++;
}

void smart::SmartAlwaysInFocus::init(unsigned char* input, int width, int height){

	preview_ratio_check( width,  height );
	__android_log_print(ANDROID_LOG_INFO, "SMART_PLANET", "layer2: do_initialization");
//	float rate = height/180.0;
//	int resize_rows = resize_frame->height   ; //(int)(height/rate);
//	int resize_cols = resize_frame->width ;  //(int)(width/rate);

//	LOGD("InitTrackROI[ h = %d , w = %d  , r = %d , c = %d , f = %f ]", height , width , rows , cols , rate);

//	int widthStride = width;

	cv::Mat gray = cv::Mat(height, width, CV_8UC1, input);
//	LOGD("InitTrackROI2[ gh = %d , gw = %d ]", gray.rows , gray.cols );
	cv::resize(gray,gray,cv::Size(resize_frame->width,resize_frame->height));
	__android_log_print(ANDROID_LOG_INFO, "SMART_PLANET", "layer2: initial dataY , w*h=[ %d , %d ]" , gray.rows , gray.cols);

//	LOGD("InitTrackROI3[ gh = %d , gw = %d ]", gray.rows , gray.cols );
	cv::Mat imCb = cv::Mat(height * 0.5, width * 0.5, CV_8UC1);
	cv::Mat imCr = cv::Mat(height * 0.5, width * 0.5, CV_8UC1);
	unsigned char *yuv420spCb = imCb.data;
	unsigned char *yuv420spCr = imCr.data;

	int start = height * width ;
	int end = start + height * width * 0.5;
	int cU = 0, cV = 0;

	for (int i = start; i < end; i += 2) {
		yuv420spCb[cU] = input[i];
		yuv420spCr[cV] = input[i + 1];
		cU++;
		cV++;
	}

//	LOGD("InitTrackROI4[ gh = %d , gw = %d ]", imCb.rows , imCb.cols );
	cv::resize(imCb,imCb,cv::Size(resize_frame->width , resize_frame->height));
	__android_log_print(ANDROID_LOG_INFO, "SMART_PLANET", "layer2: initial dataU , w*h=[ %d , %d ]" , imCb.rows , imCb.cols);

//	LOGD("InitTrackROI5[ gh = %d , gw = %d ]", imCb.rows , imCb.cols );
//	LOGD("InitTrackROI6[ gh = %d , gw = %d ]", imCr.rows , imCr.cols );
	cv::resize(imCr,imCr,cv::Size(resize_frame->width , resize_frame->height));
	__android_log_print(ANDROID_LOG_INFO, "SMART_PLANET", "layer2: initial dataR , w*h=[ %d , %d ]" , imCr.rows , imCr.cols);
//	LOGD("InitTrackROI7[ gh = %d , gw = %d ]", imCr.rows , imCr.cols );

	__android_log_print(ANDROID_LOG_INFO, "SMART_PLANET", "layer2: initial data complete");

	int tmp_rect[5];

	util::change_position_from_differnent_size(
			extern_manager->roi_data ,
			1 ,
			extern_manager->preview_width ,
			extern_manager->preview_height ,
			resize_frame->width ,
			resize_frame->height ,
			1 ,
			tmp_rect);

	cv::Rect bb;

	bb.x 	  = tmp_rect[1];
	bb.y 	  = tmp_rect[2];
	bb.width  = tmp_rect[3] - bb.x ;
	bb.height = tmp_rect[4] - bb.y ;

	__android_log_print(ANDROID_LOG_INFO, "SMART_PLANET", "layer2: bb = [ %d , %d , %d , %d ] , w*h =[ %d , %d ]" ,bb.x,bb.y,bb.width,bb.height,resize_frame->width ,resize_frame->height);

	tld_tracker->init(gray,imCb, imCr, bb, NULL);

//	is_initial = true;
//	__android_log_print(ANDROID_LOG_INFO, "SMART_PLANET", "layer2:bb = [ %d , %d , %d , %d ] , w*h =[ %d , %d ]" ,bb.x,bb.y,bb.width,bb.height,resize_frame->width ,resize_frame->height);
}

void smart::SmartAlwaysInFocus::do_tracking(unsigned char* input, int width, int height) {


	preview_ratio_check( width,  height );
	__android_log_print(ANDROID_LOG_INFO, "SMART_PLANET", "layer2: do_tracking");
//	float rate = height/180.0;
//	int resize_rows = resize_frame->height   ; //(int)(height/rate);
//	int resize_cols = resize_frame->width ;  //(int)(width/rate);

//	float rate = height/180.0;
//	int resize_rows = (int)(height/rate);
//	int resize_cols = (int)(width/rate);

	cv::Mat gray = cv::Mat(height, width, CV_8UC1, input);
	cv::resize(gray,gray,cv::Size(resize_frame->width,resize_frame->height));

	bool status = true;
	BoundingBox pbox;
	pbox.x = 0;
	pbox.y = 0;
	pbox.width = 0;
	pbox.height = 0;
	bool tl = true;

	cv::Mat imCb = cv::Mat(height * 0.5, width * 0.5, CV_8UC1);
	cv::Mat imCr = cv::Mat(height * 0.5, width * 0.5, CV_8UC1);
	unsigned char *yuv420spCb = imCb.data;
	unsigned char *yuv420spCr = imCr.data;

	int start = height * width ;
	int end = start + height * width * 0.5;
	int cU = 0, cV = 0;

	for (int i = start; i < end; i += 2) {
		yuv420spCb[cU] = input[i];
		yuv420spCr[cV] = input[i + 1];
		cU++;
		cV++;
	}

	cv::resize(imCb,imCb,cv::Size(resize_frame->width , resize_frame->height));
	cv::resize(imCr,imCr,cv::Size(resize_frame->width , resize_frame->height));

	__android_log_print(ANDROID_LOG_INFO, "SMART_PLANET", "layer2: s1 : pbox = [ %d , %d , %d , %d ] , w*h =[ %d , %d  ]" ,pbox.x,pbox.y,pbox.width,pbox.height,resize_frame->width ,resize_frame->height);

	tld_tracker->processFrame(gray,  imCb, imCr, pbox, status, tl);

	__android_log_print(ANDROID_LOG_INFO, "SMART_PLANET", "layer2: s2 : pbox = [ %d , %d , %d , %d ] , w*h =[ %d , %d  ]" ,pbox.x,pbox.y,pbox.width,pbox.height,resize_frame->width ,resize_frame->height);

	int tmp_tracked_rect[] = {pbox.x, pbox.y, pbox.width + pbox.x, pbox.height + pbox.y};

	if( pbox.x == 0  &&  pbox.width == 0 )
		is_tracked = false;
	else
		is_tracked = true;

	util::change_position_from_differnent_size(
			tmp_tracked_rect,
			0,
			resize_frame->width,
			resize_frame->height,
			extern_manager->preview_width,
			extern_manager->preview_height,
			0,
			rect_for_chaser);


	pEx_length = tld_tracker->get_pEx_size();

}


bool smart::SmartAlwaysInFocus::get_tracking_status(){
	//return ( is_tracked || is_downsize ) ;
	return  is_tracking ;
}

bool smart::SmartAlwaysInFocus::get_tracking_result(){
	return  is_tracked  ;
}

bool smart::SmartAlwaysInFocus::get_tracking_stop(){
	return  tracking_stop  ;
}

void smart::SmartAlwaysInFocus::isable_tracking_stop(){
	tracking_stop = true  ;
}

void smart::SmartAlwaysInFocus::reset_tracking_stop(){
	tracking_stop = false  ;
}

void smart::SmartAlwaysInFocus::init_clear(){
	tld_tracker->init_clear();
	__android_log_print(ANDROID_LOG_INFO, "SMART_PLANET", "layer2: init_clear. " );
}

void smart::SmartAlwaysInFocus::status_reset(){

	track_frame = 0;
	is_tracking = false ;
	is_tracked  = false ;
	memset (rect_for_chaser,-1,4);
	__android_log_print(ANDROID_LOG_INFO, "SMART_PLANET", "layer2: status_reset. " );
}

/**
* check the preview ratio is 16:9 or 4:3, adjust parameters
* @param width   preview width
* @param height  preview height
* @return        true:success to check|false:fail to check
*/
bool smart::SmartAlwaysInFocus::preview_ratio_check(const int width, const int height ){
	float preview_ratio = (float)width / height;
	if (preview_ratio > 1.72f && preview_ratio < 1.82f){
		__android_log_print(ANDROID_LOG_INFO, "SMART_PLANET", "layer2: resolution = 16:9 " );
		resize_frame->width = DownsampleWidth16_9;
		resize_frame->height = DownsampleHeight16_9;
		return true;
	}
	else if (preview_ratio > 1.28f && preview_ratio < 1.38f){
		__android_log_print(ANDROID_LOG_INFO, "SMART_PLANET", "layer2: resolution = 4:3 "  );
		resize_frame->width = DownsampleWidth4_3;
		resize_frame->height = DownsampleHeight4_3;
		return true;
	}
	else{
		__android_log_print(ANDROID_LOG_INFO, "SMART_PLANET", "layer2: resolution = do nothing "  );
		return false;
	}
}
