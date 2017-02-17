#include "smart_planet.h"

#include "smart_recognition.h"
#include "smart_composition.h"
#include "smart_always_in_focus.h"

#include <android/log.h>

smart::SmartRecognition   recognition;
smart::SmartComposition   composition;
smart::SmartAlwaysInFocus chaser;

namespace smart{

	SmartPlanet::SmartPlanet(){

		smart_sequence_index = 0;
		is_normal_sequence = true;
		for (int i = 0; i < 4; ++i){
			smart_sequence[i] = i + 1;
		}
		extern_manager = smart::SmartExternManager::get_instance();
	}

	SmartPlanet::~SmartPlanet(){
	}

    /**
     * do tracking
     * @param input  input data
     * @param width  width of input data
     * @param height height of input data
     * @return case of process
     */
	int  SmartPlanet::smart_input(unsigned char* input, int width, int height){

		int do_what = 0;
		int res_recognition = -1;

		/**
		 * to configure case according different frame index
		 * **/
		if (smart_sequence_index == 0) do_what = SMART_STATUS_RECOGNITION;
		if (smart_sequence_index == 1) do_what = SMART_STATUS_CASE3;
		if (smart_sequence_index == 2) do_what = SMART_STATUS_CASE4;

		/**
		 * force to set the case into recognition if recognition.get_tracking_mode() == true
		 * **/
		if (recognition.get_tracking_mode() == true)
		{
			do_what = SMART_STATUS_RECOGNITION;
		}

		/**
		 * force to set the case into tracking if obtain a roi from screen or chaser.get_tracking_mode() == true
		 * **/

		if( extern_manager->trigger_value == true ){
			__android_log_print(ANDROID_LOG_INFO, "SMART_PLANET", "layer 1: extern_manager->trigger_value == true");
			extern_manager->trigger_value = false;
			chaser.init_clear();
			chaser.status_reset();
			chaser.isable_tracking_stop();
			smart_output_alwaysinfocus();
		}

		if (extern_manager->is_roi_selected == true ) {
			__android_log_print(ANDROID_LOG_INFO, "SMART_PLANET", "layer 1: extern_manager->is_roi_selected == true");
			do_what = SMART_STATUS_ALWAYINFOCUS;
			extern_manager->is_roi_selected = false;
			chaser.status_reset();

		}

		if( chaser.get_tracking_status() == true ){
			do_what = SMART_STATUS_ALWAYINFOCUS;
			__android_log_print(ANDROID_LOG_INFO, "SMART_PLANET", "layer 1: do_what = SMART_STATUS_ALWAYINFOCUS");

		}

		if( chaser.get_tracking_stop() == true ){
			__android_log_print(ANDROID_LOG_INFO, "SMART_PLANET", "layer 1: _.get_tracking_stop() == true");
			chaser.reset_tracking_stop();
			do_what = SMART_STATUS_CASE3;
		}


		smart_sequence_index++;
		if (smart_sequence_index >= 3)
			smart_sequence_index = 0;


		/**
		 * run every case in turn
		 * **/
		//do_what = SMART_STATUS_ALWAYINFOCUS;
		switch (do_what)
		{
		case SMART_STATUS_RECOGNITION:
			__android_log_print(ANDROID_LOG_INFO, "SMART_PLANET", "CASE 1 : SMART_STATUS_RECOGNITION");

//			res_recognition = recognition.detect_scene(input, width, height);
//			smart_output_recognition();
//			smart_output_composition(res_recognition);
			break;
		case SMART_STATUS_ALWAYINFOCUS:
			__android_log_print(ANDROID_LOG_INFO, "SMART_PLANET", "CASE 2 : SMART_STATUS_ALWAYINFOCUS");

			//chaser.detect_scene(input, width, height);
			chaser.process_frame(input, width, height);
			smart_output_alwaysinfocus();
			break;
		case SMART_STATUS_CASE3:
			__android_log_print(ANDROID_LOG_INFO, "SMART_PLANET", "CASE 3 : SMART_STATUS_CASE3");
			break;
		case SMART_STATUS_CASE4:
			__android_log_print(ANDROID_LOG_INFO, "SMART_PLANET", "CASE 4 : SMART_STATUS_CASE4");
			break;
		default:
			break;
		}

        __android_log_print(ANDROID_LOG_INFO, "SMART_PLANET", "do what = %d" , do_what);
		return do_what;

	}

	/**
	 * obtain the result from smart_recognition
	 * @param
	 * @return
	 */
	void SmartPlanet::smart_output_recognition(){
		int* scenesType = recognition.get_scene_flag();
		output_container.getScenesResult().scenes[0] = scenesType[0];
		output_container.getScenesResult().scenes[1] = scenesType[1];
		output_container.getScenesResult().allow = true;
		__android_log_print(ANDROID_LOG_INFO, "SMART_KIT", "smart_planet_recognition_output flag1 = %d , flag2 = %d" , output_container.getScenesResult().scenes[0] , output_container.getScenesResult().scenes[1] );

		output_container.getQRCodeParam().rect[0] = recognition.tracked_rect_for_gl_matrix[0];
		output_container.getQRCodeParam().rect[1] = recognition.tracked_rect_for_gl_matrix[1];
		output_container.getQRCodeParam().rect[2] = recognition.tracked_rect_for_gl_matrix[2];
		output_container.getQRCodeParam().rect[3] = recognition.tracked_rect_for_gl_matrix[3];
		output_container.getQRCodeParam().rect[4] = recognition.tracked_rect_for_gl_matrix[4];
		output_container.getQRCodeParam().rect[5] = recognition.tracked_rect_for_gl_matrix[5];
		output_container.getQRCodeParam().rect[6] = recognition.tracked_rect_for_gl_matrix[6];
		output_container.getQRCodeParam().rect[7] = recognition.tracked_rect_for_gl_matrix[7];
		output_container.getQRCodeParam().allow = true;
		output_container.getQRCodeParam().code_format = recognition.get_code_format();
		output_container.getQRCodeParam().code_str    = recognition.get_qrcode_decoder_str();
		output_container.getQRCodeParam().code_type   = recognition.get_qrcode_decoder_type();
	}

	/**
	 * set the threshold for tuning composition
	 * @param paramI   int type parameters
	 * @param len1     length of param1
	 * @param paramF   float type parameters
	 * @param len2     length of param2
	 * @return         true:success to tuning|false:fail to tuning
	 */
	bool SmartPlanet::smart_config_composition(void* paramI, int len1, void* paramF, int len2){
		int  res  = composition.setSmartParameter(paramI, len1, paramF, len2);
		bool res2 = recognition.setSmartParameter(paramI, len1, paramF, len2);
		return res;
	}

	/**
	 * obtain the result from smart_composition
	 * @param flag  type of scenes
	 * @return   1 :success to tuning| 2 :fail to tuning
	 */
	int  SmartPlanet::smart_output_composition(int flag)
	{

		if (!recognition.is_ready_for_guide) {
			output_container.getGuideParam().reset();
			return 0;
		}
		else{
			output_container.getGuideParam().allow = true;
		}

		GuideInputParam input;
		input.rect[0] = recognition.tracked_rect_for_guide[0];
		input.rect[1] = recognition.tracked_rect_for_guide[1];
		input.rect[2] = recognition.tracked_rect_for_guide[2];
		input.rect[3] = recognition.tracked_rect_for_guide[3];
		input.center_x = recognition.tracked_rect_for_guide[4];
		input.center_y = recognition.tracked_rect_for_guide[5];
		input.orientation = extern_manager->orientation;
		input.orientation_raw = extern_manager->orientation_raw;
		input.width = recognition.process_image_width;
		input.height = recognition.process_image_height;
		input.preview_width = extern_manager->preview_width;
		input.preview_height = extern_manager->preview_height;
		input.screen_width = extern_manager->screen_width;
		input.screen_height = extern_manager->screen_height;

		GuideParam guideParam;
		composition.compose_scene(flag, (void*)&input, (void*)&guideParam);

		switch (flag){
		case FLAG_DETECTION_PORTRAIT:
			output_container.getGuideParam().getPortrait().allow 				= guideParam.getPortrait().isAllow();               // composData[1] = guideParam.getPortrait().isAllow() == true ? 1 : 0;
			output_container.getGuideParam().getPortrait().zoomRatio 			= guideParam.getPortrait().getZoomRatio();          // composData[2] = guideParam.getPortrait().getZoomRatio();
			output_container.getGuideParam().getPortrait().zoomFlag 			= guideParam.getPortrait().getZoomFlag();           // composData[3] = guideParam.getPortrait().getZoomFlag();
			output_container.getGuideParam().getPortrait().currentPosition.x 	= guideParam.getPortrait().getCurrentPosition().x;  // composData[4] = guideParam.getPortrait().getCurrentPosition().x;
			output_container.getGuideParam().getPortrait().currentPosition.y 	= guideParam.getPortrait().getCurrentPosition().y;  // composData[5] = guideParam.getPortrait().getCurrentPosition().y;
			output_container.getGuideParam().getPortrait().orientation 			= guideParam.getPortrait().getOrientation();        // composData[6] = guideParam.getPortrait().getOrientation();
			output_container.getGuideParam().getPortrait().guideFlag 			= guideParam.getPortrait().getGuideFlag();          // composData[7] = guideParam.getPortrait().getGuideFlag();
			output_container.getGuideParam().getPortrait().leftRatio 			= guideParam.getPortrait().getTargetRatio()[0];     // composData[8] = guideParam.getPortrait().getTargetRatio()[0];
			output_container.getGuideParam().getPortrait().rightRatio 			= guideParam.getPortrait().getTargetRatio()[1];     //  composData[9] = guideParam.getPortrait().getTargetRatio()[1];
			break;
		case FLAG_DETECTION_FOOD:

			output_container.getGuideParam().getFood().allow 					= guideParam.getFood().isAllow();					// composData[1] = guideParam.getFood().isAllow() == true ? 1 : 0;
			output_container.getGuideParam().getFood().zoomRatio 				= guideParam.getFood().getZoomRatio();				// composData[2] = guideParam.getFood().getZoomRatio();
			output_container.getGuideParam().getFood().zoomFlag 				= guideParam.getFood().getZoomFlag();				// composData[3] = guideParam.getFood().getZoomFlag();
			output_container.getGuideParam().getFood().currentPosition.x 		= guideParam.getFood().getCurrentPosition().x;		// composData[4] = guideParam.getFood().getCurrentPosition().x;
			output_container.getGuideParam().getFood().currentPosition.y 		= guideParam.getFood().getCurrentPosition().y;		// composData[5] = guideParam.getFood().getCurrentPosition().y;
			output_container.getGuideParam().getFood().guideFlag 				= guideParam.getFood().getGuideFlag();				// composData[7] = guideParam.getFood().getGuideFlag();
			output_container.getGuideParam().getFood().targetPosition.x 		= guideParam.getFood().getTargetPosition().x;		// composData[12] = guideParam.getFood().getTargetPosition().x;
			output_container.getGuideParam().getFood().targetPosition.y 		= guideParam.getFood().getTargetPosition().y;		// composData[13] =  guideParam.getFood().getTargetPosition().y;
			output_container.getGuideParam().getFood().food_attractiveness      = recognition.get_food_attractiveness();
			break;
		case FLAG_DETECTION_LANDSCAPE:
			output_container.getGuideParam().getLandscape().allow 				= guideParam.getLandscape().isAllow();						  // composData[1] = guideParam.getLandscape().isAllow() == true ? 1 : 0;
			output_container.getGuideParam().getLandscape().levelAngle 			= guideParam.getLandscape().getLevelAngle();				  // composData[2] = guideParam.getLandscape().getLevelAngle();
			output_container.getGuideParam().getLandscape().currentPosition.x 	= guideParam.getLandscape().getCurrentPosition().x; // composData[4] = guideParam.getLandscape().getCurrentPosition().x;
			output_container.getGuideParam().getLandscape().currentPosition.y 	= guideParam.getLandscape().getCurrentPosition().y; // composData[5] = guideParam.getLandscape().getCurrentPosition().y;
			output_container.getGuideParam().getLandscape().orientation 		= guideParam.getLandscape().getOrientation();		// composData[6] = guideParam.getLandscape().getOrientation();
			output_container.getGuideParam().getLandscape().guideFlag			= guideParam.getLandscape().getGuideFlag(); // composData[7] = guideParam.getLandscape().getGuideFlag();
			output_container.getGuideParam().getLandscape().distanceOk 			= guideParam.getLandscape().isDistanceOk(); // composData[10] = guideParam.getLandscape().isDistanceOk() == true ? 1 : 0;
			output_container.getGuideParam().getLandscape().angleFlag 			= guideParam.getLandscape().getAngleflag(); //  composData[11] = guideParam.getLandscape().getAngleflag() == true ? 1 : 0;
			output_container.getGuideParam().getLandscape().targetPosition.x 	= guideParam.getLandscape().getTargetPosition().x;  // composData[12] = guideParam.getLandscape().getTargetPosition().x;
			output_container.getGuideParam().getLandscape().targetPosition.y 	= guideParam.getLandscape().getTargetPosition().y;  // composData[13] = guideParam.getLandscape().getTargetPosition().y;
			break;
		default:
			break;
		}

		return 1;
	}

	/**
	 * obtain the result from smart_always_in_focus
	 * @param
	 * @return
	 */
	void SmartPlanet::smart_output_alwaysinfocus(){
		output_container.getTrackParam().allow   = chaser.get_tracking_result();
		output_container.getTrackParam().rect[0] = chaser.rect_for_chaser[0];
		output_container.getTrackParam().rect[1] = chaser.rect_for_chaser[1];
		output_container.getTrackParam().rect[2] = chaser.rect_for_chaser[2];
		output_container.getTrackParam().rect[3] = chaser.rect_for_chaser[3];
		output_container.getTrackParam().pEx_length = chaser.pEx_length ;
		__android_log_print(ANDROID_LOG_INFO, "SMART_PLANET", "layer 1: alwaysinfocus_output: [ %d , %d , %d , %d ]" ,
				chaser.rect_for_chaser[0],
				chaser.rect_for_chaser[1],
				chaser.rect_for_chaser[2],
				chaser.rect_for_chaser[3]);
	}

    /**
     * output
     * @param OutputParam**  address of pointer of OutputParam Structure
     * @return
     */
	void SmartPlanet::smart_output(OutputParam** output){

		*output = &output_container;
//		__android_log_print(ANDROID_LOG_INFO, "SMART_KIT", "smart_planet_get_result_* flag1 = %d , flag2 = %d" , _output->getScenesResult().scenes[0] , _output->getScenesResult().scenes[1] );

	}

}
