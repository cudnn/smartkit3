/*
 * smart_kit_jni.cpp
 *
 *  Created on: 2015/07/28
 *      Author: guoyj7
 *
 * This file defined the smartkit JNI interfaces, which are called by the app layer
 * The code below can also be seem as the sample code for integration of HAL layer
 *
 *  Revised on: 2016/03/31
 *      Author: fengbin1
 *  Revised on: 2016/07/11
 *      Author: xufeng8
 */
#define _ANDROID_
#define DEBUG

#include <iostream>
#include <jni.h>
#include <android/log.h>//#include "log.h"

#include "smart_extern_manager.h"
#include "smart_planet.h"


int composData[64];
//数组索引对应的变量含义
// 索引     变量含义                                                 构图对应场景
//	0	场景识别flag
//	1	场景构图允许Flag	                        人像/食物/风景
//	2	目标区域/理想区域（地平线角度）     人像/食物/风景
//	3	远近达标Flag                人像/食物
//	4	目标坐标（X）                                       人像/食物/风景
//	5	目标坐标（Y）                                       人像/食物/风景
//	6	屏幕方向                                                 人像/风景
//	7	构图达标Flag                人像/食物/风景
//	8	屏幕左边目标区域亮度                           人像
//	9	屏幕右边目标区域亮度                           人像
//	10	高度达标Flag                风景
//	11	角度达标Flag                风景
//	12	引导位置（X）                                        食物/风景
//	13	引导位置（Y）                                        食物/风景


using namespace smart;
//SmartAR smart_ar;
//SmartRecognition recognition;
//SmartComposition composition;
SmartExternManager* externManager = SmartExternManager::get_instance();
SmartPlanet planet;

int guide_v2();
//set the preview data size
extern "C" JNIEXPORT jint Java_com_lenovo_smartengine_jni_SmartNative_setPreviewSize( JNIEnv* env,
        jobject thiz,
        jint width,
        jint height)
{
	__android_log_print(ANDROID_LOG_INFO, "SMART_KIT_JNI", "%s E" , __func__);
	externManager->set_preview_size(width, height);
//	smart_ar.init_camera_param();

	__android_log_print(ANDROID_LOG_INFO, "SMART_KIT_JNI", "%s X" , __func__);
    return 0;
}

//set the camera screen size
extern "C" JNIEXPORT jint Java_com_lenovo_smartengine_jni_SmartNative_setScreenSize( JNIEnv* env,
        jobject thiz,
        jint width,
        jint height)
{
	__android_log_print(ANDROID_LOG_INFO, "SMART_KIT_JNI", "%s E" , __func__);

	externManager->set_screen_size(width, height);

	__android_log_print(ANDROID_LOG_INFO, "SMART_KIT_JNI", "%s X" , __func__);
    return 0;
}

//set the faces data
extern "C" JNIEXPORT jint Java_com_lenovo_smartengine_jni_SmartNative_setFaceData( JNIEnv* env,
        jobject thiz,
        jintArray data,
        jint length)
{
	__android_log_print(ANDROID_LOG_INFO, "SMART_KIT_JNI", "%s E" , __func__);
	jint* faceData = env->GetIntArrayElements(data, NULL);

	externManager->update_face(faceData, length);

    env->ReleaseIntArrayElements(data, faceData, 0);
	__android_log_print(ANDROID_LOG_INFO, "SMART_KIT_JNI", "%s X" , __func__);
    return 0;
}

//set the tuning parameters
extern "C" JNIEXPORT jint Java_com_lenovo_smartengine_jni_SmartNative_setSmartParameter( JNIEnv* env,
        jobject thiz,
        jintArray param1,
        jint len1,
        jfloatArray param2,
        jint len2)
{
	__android_log_print(ANDROID_LOG_INFO, "SMART_KIT_JNI", "%s E" , __func__);
	jint* paramData1 = env->GetIntArrayElements(param1, NULL);
	jfloat* paramData2 = env->GetFloatArrayElements(param2, NULL);

//	composition.setSmartParameter(paramData1, len1, paramData2, len2);
	int set_res = planet.smart_config_composition(paramData1, len1, paramData2, len2);
    env->ReleaseIntArrayElements(param1, paramData1, 0);
    env->ReleaseFloatArrayElements(param2, paramData2, 0);
	__android_log_print(ANDROID_LOG_INFO, "SMART_KIT_JNI", "%s X" , __func__);
    return 0;
}

//set the tuning parameters
extern "C" JNIEXPORT jint Java_com_lenovo_smartengine_jni_SmartNative_setTrigger( JNIEnv* env,
        jobject thiz,
        jboolean trigger_value)
{
	__android_log_print(ANDROID_LOG_INFO, "SMART_KIT_JNI", "%s E" , __func__);

	externManager->set_trigger(trigger_value);

	__android_log_print(ANDROID_LOG_INFO, "SMART_KIT_JNI", "%s E" , __func__);
    return 0;
}

//set the isp data
extern "C" JNIEXPORT jint Java_com_lenovo_smartengine_jni_SmartNative_setISPData( JNIEnv* env,
        jobject thiz,
        jfloatArray data,
        jint length)
{
	__android_log_print(ANDROID_LOG_INFO, "SMART_KIT_JNI", "%s E" , __func__);
	jfloat* ispData = env->GetFloatArrayElements(data, NULL);

	externManager->update_isp(ispData, length);

    env->ReleaseFloatArrayElements(data, ispData, 0);
	__android_log_print(ANDROID_LOG_INFO, "SMART_KIT_JNI", "%s X" , __func__);
    return 0;
}

//set the orientation sensor data
extern "C" JNIEXPORT jint Java_com_lenovo_smartengine_jni_SmartNative_setOsensorData( JNIEnv* env,
        jobject thiz,
        jfloatArray data,
        jint length)
{
	__android_log_print(ANDROID_LOG_INFO, "SMART_KIT_JNI", "%s E" , __func__);
	jfloat* osensor = env->GetFloatArrayElements(data, NULL);

	externManager->update_sensor(SMART_SENSOR_TYPE_ORIENTATION, osensor, length);

    env->ReleaseFloatArrayElements(data, osensor, 0);
	__android_log_print(ANDROID_LOG_INFO, "SMART_KIT_JNI", "%s X" , __func__);
    return 0;
}

//set the accelerometer sensor data
extern "C" JNIEXPORT jint Java_com_lenovo_smartengine_jni_SmartNative_setAsensorData( JNIEnv* env,
        jobject thiz,
        jfloatArray data,
        jint length)
{
	__android_log_print(ANDROID_LOG_INFO, "SMART_KIT_JNI", "%s E" , __func__);
	jfloat* asensor = env->GetFloatArrayElements(data, NULL);

	externManager->update_sensor(SMART_SENSOR_TYPE_ACCELEROMETER, asensor, length);

    env->ReleaseFloatArrayElements(data, asensor, 0);
	__android_log_print(ANDROID_LOG_INFO, "SMART_KIT_JNI", "%s X" , __func__);
    return 0;
}

//set the light sensor data
extern "C" JNIEXPORT jint Java_com_lenovo_smartengine_jni_SmartNative_setLsensorData( JNIEnv* env,
        jobject thiz,
        jfloatArray data,
        jint length)
{
	__android_log_print(ANDROID_LOG_INFO, "SMART_KIT_JNI", "%s E" , __func__);
	jfloat* lsensor = env->GetFloatArrayElements(data, NULL);

	externManager->update_sensor(SMART_SENSOR_TYPE_LIGHT, lsensor, length);

    env->ReleaseFloatArrayElements(data, lsensor, 0);
	__android_log_print(ANDROID_LOG_INFO, "SMART_KIT_JNI", "%s X" , __func__);
    return 0;
}

//set the orientation of camera
extern "C" JNIEXPORT jint Java_com_lenovo_smartengine_jni_SmartNative_setOrientation( JNIEnv* env,
        jobject thiz,
        jint ori)
{
	__android_log_print(ANDROID_LOG_INFO, "SMART_KIT_JNI", "%s E" , __func__);

	externManager->update_orientation(ori);

	__android_log_print(ANDROID_LOG_INFO, "SMART_KIT_JNI", "%s X" , __func__);
    return 0;
}

//set the faces data
extern "C" JNIEXPORT jint Java_com_lenovo_smartengine_jni_SmartNative_setROIData( JNIEnv* env,
        jobject thiz,
        jintArray data,
        jint length)
{
	__android_log_print(ANDROID_LOG_INFO, "SMART_KIT_JNI", "%s E" , __func__);
	jint* roiData = env->GetIntArrayElements(data, NULL);

	externManager->update_roi(roiData, length);

    env->ReleaseIntArrayElements(data, roiData, 0);
	__android_log_print(ANDROID_LOG_INFO, "SMART_KIT_JNI", "%s X" , __func__);
    return 0;
}


//smart recognition and composition process
extern "C" JNIEXPORT jint Java_com_lenovo_smartengine_jni_SmartNative_sceneDetector( JNIEnv* env,
        jobject thiz,
        jbyteArray data)
{
	__android_log_print(ANDROID_LOG_INFO, "SMART_KIT_JNI", "%s E" , __func__);
	jbyte* yuvframe = env->GetByteArrayElements(data, NULL);

//	int flag = recognition.detect_scene((unsigned char *)yuvframe, externManager->preview_width, externManager->preview_height);
//	guide(flag);
	int flag = planet.smart_input((unsigned char *)yuvframe, externManager->preview_width, externManager->preview_height);
	guide_v2();

//	SMART_DEBUG("%s detect_scene flag = %d", __func__, flag);

//	smart_ar.calculate_mvp_matrix(flag, recognition.is_ready_for_gl_model, recognition.tracked_rect_for_gl_matrix, 4, 2, 0, 0, 1);



    env->ReleaseByteArrayElements(data, yuvframe, 0);

	__android_log_print(ANDROID_LOG_INFO, "SMART_KIT_JNI", "%s X" , __func__);
    return flag;
}

int guide_v2( )
{

	OutputParam* outputParam = NULL;
	planet.smart_output(&outputParam);	// output

	composData[0] = outputParam->getScenesResult().scenes[0];
	composData[1] = 0;

//				__android_log_print(ANDROID_LOG_INFO, "SMART_PLANET", "food = [ %d , %d , %d , %d , %d , %d , %d ,%d]" ,
//						outputParam->getGuideParam().getFood().isAllow() ,
//						outputParam->getGuideParam().getFood().getZoomRatio() ,
//						outputParam->getGuideParam().getFood().getZoomFlag() ,
//						outputParam->getGuideParam().getFood().getCurrentPosition().x,
//						outputParam->getGuideParam().getFood().getCurrentPosition().y,
//						outputParam->getGuideParam().getFood().getGuideFlag(),
//						outputParam->getGuideParam().getFood().getTargetPosition().x,
//						outputParam->getGuideParam().getFood().getTargetPosition().y);
//
//				__android_log_print(ANDROID_LOG_INFO, "SMART_PLANET", "portrait = [ %d , %d , %d , %d , %d , %d , %d ,%d,%d]" ,
//						outputParam->getGuideParam().getPortrait().isAllow() ,
//						outputParam->getGuideParam().getPortrait().getZoomRatio() ,
//						outputParam->getGuideParam().getPortrait().getZoomFlag() ,
//						outputParam->getGuideParam().getPortrait().getCurrentPosition().x,
//						outputParam->getGuideParam().getPortrait().getCurrentPosition().y,
//						outputParam->getGuideParam().getPortrait().getOrientation(),
//						outputParam->getGuideParam().getPortrait().getGuideFlag(),
//						outputParam->getGuideParam().getPortrait().getTargetRatio()[0],
//						outputParam->getGuideParam().getPortrait().getTargetRatio()[1]);
//
//				__android_log_print(ANDROID_LOG_INFO, "SMART_PLANET", "landscape = [ %d , %d , %d , %d , %d , %d , %d ,%d ,%d , %d]" ,
//						outputParam->getGuideParam().getLandscape().isAllow(),
//						outputParam->getGuideParam().getLandscape().getLevelAngle() ,
//						outputParam->getGuideParam().getLandscape().getCurrentPosition().x,
//						outputParam->getGuideParam().getLandscape().getCurrentPosition().y,
//						outputParam->getGuideParam().getLandscape().getOrientation(),
//						outputParam->getGuideParam().getLandscape().getGuideFlag(),
//						outputParam->getGuideParam().getLandscape().isDistanceOk(),
//						outputParam->getGuideParam().getLandscape().getAngleflag(),
//						outputParam->getGuideParam().getLandscape().getTargetPosition().x,
//						outputParam->getGuideParam().getLandscape().getTargetPosition().y);

	switch(outputParam->getScenesResult().scenes[0]){
		case FLAG_DETECTION_PORTRAIT:
			composData[1]  = outputParam->getGuideParam().getPortrait().isAllow()== true ? 1 : 0;
			composData[2]  = outputParam->getGuideParam().getPortrait().getZoomRatio();
			composData[3]  = outputParam->getGuideParam().getPortrait().getZoomFlag();
			composData[4]  = outputParam->getGuideParam().getPortrait().getCurrentPosition().x;
			composData[5]  = outputParam->getGuideParam().getPortrait().getCurrentPosition().y;
			composData[6]  = outputParam->getGuideParam().getPortrait().getOrientation();
			composData[7]  = outputParam->getGuideParam().getPortrait().getGuideFlag();
			composData[8]  = outputParam->getGuideParam().getPortrait().getTargetRatio()[0];
			composData[9]  = outputParam->getGuideParam().getPortrait().getTargetRatio()[1];
			break;
		case FLAG_DETECTION_FOOD:
			composData[1]  = outputParam->getGuideParam().getFood().isAllow() == true ? 1 : 0;
			composData[2]  = outputParam->getGuideParam().getFood().getZoomRatio();
			composData[3]  = outputParam->getGuideParam().getFood().getZoomFlag();
			composData[4]  = outputParam->getGuideParam().getFood().getCurrentPosition().x;
			composData[5]  = outputParam->getGuideParam().getFood().getCurrentPosition().y;
			composData[7]  = outputParam->getGuideParam().getFood().getGuideFlag();
			composData[12] = outputParam->getGuideParam().getFood().getTargetPosition().x;
			composData[13] = outputParam->getGuideParam().getFood().getTargetPosition().y;
			break;
		case FLAG_DETECTION_LANDSCAPE:
			composData[1]  = outputParam->getGuideParam().getLandscape().isAllow() == true ? 1 : 0;
			composData[2]  = outputParam->getGuideParam().getLandscape().getLevelAngle();
			composData[4]  = outputParam->getGuideParam().getLandscape().getCurrentPosition().x;
			composData[5]  = outputParam->getGuideParam().getLandscape().getCurrentPosition().y;
			composData[6]  = outputParam->getGuideParam().getLandscape().getOrientation();
			composData[7]  = outputParam->getGuideParam().getLandscape().getGuideFlag();
			composData[10] = outputParam->getGuideParam().getLandscape().isDistanceOk() == true ? 1 : 0;
			composData[11] = outputParam->getGuideParam().getLandscape().getAngleflag() == true ? 1 : 0;
			composData[12] = outputParam->getGuideParam().getLandscape().getTargetPosition().x;
			composData[13] = outputParam->getGuideParam().getLandscape().getTargetPosition().y;
			break;
		default:
			break;
	}

	if( outputParam->getTrackParam().isAllow() == true ){

		composData[20] = 1;

		composData[21] = outputParam->getTrackParam().rect[0];
		composData[22] = outputParam->getTrackParam().rect[1];
		composData[23] = outputParam->getTrackParam().rect[2];
		composData[24] = outputParam->getTrackParam().rect[3];
		composData[25] = outputParam->getTrackParam().pEx_length;
	}else{
		composData[20] = 0;
	}

	return 1;

//	SMART_DEBUG("guide_portrait : scene_flag = %d, scene_guide_allow = %d, ratio_angle = %d, zoom_flag = %d, current_position_x = %d, current_position_y = %d, screen_orientation = %d, scene_guide_flag = %d, target_transparency_left = %d, target_transparency_right = %d, is_height_ok = %d, is_angle_ok = %d",
//			composData[0], composData[1], composData[2], composData[3], composData[4], composData[5], composData[6], composData[7], composData[8], composData[9], composData[10], composData[11]);
}




//get the results of composition guide
extern "C" JNIEXPORT jintArray Java_com_lenovo_smartengine_jni_SmartNative_getCompositeResult( JNIEnv* env,
        jobject thiz)
 {
	__android_log_print(ANDROID_LOG_INFO, "SMART_KIT_JNI", "%s E" , __func__);
	jintArray jniDebugData = env->NewIntArray(64);

	env->SetIntArrayRegion(jniDebugData, 0, 64, composData);

	__android_log_print(ANDROID_LOG_INFO, "SMART_KIT_JNI", "%s X" , __func__);
	return jniDebugData;
}

//get the results of scene recognition
extern "C" JNIEXPORT jintArray Java_com_lenovo_smartengine_jni_SmartNative_getSceneResult( JNIEnv* env,
        jobject thiz)
 {
	__android_log_print(ANDROID_LOG_INFO, "SMART_KIT_JNI", "%s E" , __func__);
	jintArray jniSceneResult = env->NewIntArray(2);

//	int* sceneResult = recognition.get_scene_flag();

	int sceneResult[2];

	OutputParam* outputParam = NULL;
	planet.smart_output(&outputParam);	// output
//	OutputParam outputParam;
//	planet.smart_get_result((void*) &outputParam);	// output

	sceneResult[0] = outputParam->getScenesResult().scenes[0] ;
	sceneResult[1] = outputParam->getScenesResult().scenes[1] ;

	env->SetIntArrayRegion(jniSceneResult, 0, 2, sceneResult);

//	__android_log_print(ANDROID_LOG_INFO, "SMART_KIT", "smart_jni_recognition_output flag1 = %d , flag2 = %d" , outputParam->getScenesResult().scenes[0] , outputParam->getScenesResult().scenes[1] );

	return jniSceneResult;

	__android_log_print(ANDROID_LOG_INFO, "SMART_KIT_JNI", "%s X" , __func__);
	return 0;
}

//get the debugData of recognition module
extern "C" JNIEXPORT jfloatArray Java_com_lenovo_smartengine_jni_SmartNative_getDebugValue( JNIEnv* env,
        jobject thiz)
 {
	__android_log_print(ANDROID_LOG_INFO, "SMART_KIT_JNI", "%s E" , __func__);
	jfloatArray jniDebugvalue = env->NewFloatArray(32);
//	env->SetFloatArrayRegion(jniDebugvalue, 0, 32, recognition.debug_value);
	__android_log_print(ANDROID_LOG_INFO, "SMART_KIT_JNI", "%s X" , __func__);
	return jniDebugvalue;
}

//get the AR transformation matrix
extern "C" JNIEXPORT jfloatArray Java_com_lenovo_smartengine_jni_SmartNative_getMVPMatrix( JNIEnv* env,
        jobject thiz
       )
{
//	float *projectionMatrix = smart_ar.get_mvp_matrix();
	float projectionMatrix[16]  ;
	jfloatArray jniProjectionMat = env->NewFloatArray(16);
	env->SetFloatArrayRegion(jniProjectionMat, 0, 16, projectionMatrix);
	return jniProjectionMat;
}

//get the AR extrinsic matrix
extern "C" JNIEXPORT jfloatArray Java_com_lenovo_smartengine_jni_SmartNative_getExtrinsicMatrix( JNIEnv* env,
        jobject thiz
       )
{
//	float *projectionMatrix = smart_ar.get_extrinsic_matrix();
	float projectionMatrix[16];
	jfloatArray jniProjectionMat = env->NewFloatArray(16);
	env->SetFloatArrayRegion(jniProjectionMat, 0, 16, projectionMatrix);
	return jniProjectionMat;
}

//get the AR projection matrix
extern "C" JNIEXPORT jfloatArray Java_com_lenovo_smartengine_jni_SmartNative_getGlProjectionMatrix( JNIEnv* env,
        jobject thiz
       )
{
//	float *projectionMatrix = smart_ar.get_gl_projection_matrix();
	float projectionMatrix[16];
	jfloatArray jniProjectionMat = env->NewFloatArray(16);
	env->SetFloatArrayRegion(jniProjectionMat, 0, 16, projectionMatrix);
	return jniProjectionMat;
}

//get the position of QRCode
extern "C" JNIEXPORT jfloatArray  Java_com_lenovo_smartengine_jni_SmartNative_getQRPostion( JNIEnv* env,
        jobject thiz)
 {
	__android_log_print(ANDROID_LOG_INFO, "SMART_KIT_JNI", "%s E" , __func__);
	jfloatArray jniQRPos = env->NewFloatArray(8);

//	env->SetFloatArrayRegion(jniQRPos, 0, 8, recognition.tracked_rect_for_gl_matrix);

	OutputParam* outputParam = NULL;
	planet.smart_output(&outputParam);	// output
	env->SetFloatArrayRegion(jniQRPos, 0, 8,outputParam->getQRCodeParam().rect );
	__android_log_print(ANDROID_LOG_INFO, "SMART_KIT_JNI", "%s X" , __func__);
	return jniQRPos;
 }

//get the decode results of QRCode
extern "C" JNIEXPORT jstring  Java_com_lenovo_smartengine_jni_SmartNative_getQRCodeDecodeTxt( JNIEnv* env,
        jobject thiz)
 {
	OutputParam* outputParam = NULL;
	planet.smart_output(&outputParam);
//	SMART_DEBUG("get_qrcode_decoder_str %s", outputParam->getQRCodeParam().code_str.c_str() );
	__android_log_print(ANDROID_LOG_INFO, "SMART_KIT_JNI", "get_qrcode_decoder_str %s" , outputParam->getQRCodeParam().code_str.c_str());
	char* temp = (char*)outputParam->getQRCodeParam().code_str.c_str(); //(char*)(recognition.get_qrcode_decoder_str().c_str());
	return env->NewStringUTF((const char*)temp);
 }

//get the decode type of QRCode
extern "C" JNIEXPORT jstring  Java_com_lenovo_smartengine_jni_SmartNative_getQRCodeDecodeType( JNIEnv* env,
        jobject thiz)
 {
	OutputParam* outputParam = NULL;
	planet.smart_output(&outputParam);
	char* temp = (char*)outputParam->getQRCodeParam().code_type.c_str();  //= (char*)(recognition.get_qrcode_decoder_type().c_str());
	return env->NewStringUTF((const char*)temp);
 }

//get the attractiveness of food
extern "C" JNIEXPORT jfloat  Java_com_lenovo_smartengine_jni_SmartNative_getFoodAttractiveness( JNIEnv* env,
        jobject thiz)
 {
	OutputParam* outputParam = NULL;
	planet.smart_output(&outputParam);
	float attractive_score = outputParam->getGuideParam().getFood().getFoodAttractiveness();
//	SMART_DEBUG("attractive_score %.2f", attractive_score);
	return attractive_score;
 }
