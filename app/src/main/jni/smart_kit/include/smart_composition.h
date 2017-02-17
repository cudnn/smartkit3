/*
 * smart_compos.h
 *
 *  Created on: 2015年7月27日
 *      Author: guoyj7
 *
 * 	Add comments on variable and functions, clear up the unused part
 *
 *  Revised on: 2016/04/8
 *      Author: fengbin1
 */

#ifndef SMART_KIT_SMART_COMPOSITION_H_
#define SMART_KIT_SMART_COMPOSITION_H_

#include "smart_type.h"

namespace smart {

class SmartComposition {
public:
	SmartComposition();
	virtual ~SmartComposition();

    /**
     * enable the specific scene composition guide
     * @param bit  scene type (defined in include/smart_type.h)
     * @return
     */
	bool enable_composition(int bit);
    /**
     * disable the specific scene composition guide
     * @param bit  scene type (defined in include/smart_type.h)
     * @return
     */
	bool disable_composition(int bit);
    /**
     * judge whether the specific scene composition guide is enabled
     * @param bit  scene type (defined in include/smart_type.h)
     * @return     true:enable|false:disable
     */
	bool is_enable(const int bit);
	/**
	 * set the threshold for tuning composition
	 * @param param1   int type parameters
	 * @param len1     length of param1
	 * @param param2   float type parameters
	 * @param len2     length of param2
	 * @return         true:success to tuning|false:fail to tuning
	 */
	bool setSmartParameter(void* paramI, int len1, void* paramF, int len2);
    /**
     * do scene composition guide
     * @param scene   scene flag, which scene to be composition
     * @param input   input data for composition process
     * @param output  output data for ICG UI design
     * @return
     */
	void  compose_scene(int scene, void* input, void* output);


private:
    /**
     * do portrait composition guide
     * @param input   input data for composition process
     * @param output  output data for ICG UI design
     * @return
     */
	int compose_portrait(GuideInputParam* input, GuideParam* output);
    /**
     * do food composition guide
     * @param input   input data for composition process
     * @param output  output data for ICG UI design
     * @return
     */
	int compose_food(GuideInputParam* input, GuideParam* output);
    /**
     * do line composition guide
     * @param input   input data for composition process
     * @param output  output data for ICG UI design
     * @return
     */
	int compose_line(GuideInputParam* input, GuideParam* output);
    /**
     * reset composition data
     * @param scene   scene flag, which scene to be composition
     * @param output  output data for ICG UI design
     * @return
     */
	void compose_reset(int scene ,  GuideParam* output);

	int Face_ThresLow;          //lower threshold of portrait ratio, default:100
	int Face_ThresHigh;         //upper threshold of portrait ratio, default:120
	float Face_ThresGood;       //"Good" threshold of portrait guide, default:0.06f
	float Face_ThresPerfect;    //"Perfect" threshold of portrait guide, default:0.02f
	float FaceHeight;           //threshold of normal portrait height, default:0.2f
	float FaceTrigger;          //threshold of ROI for portrait guide, default:2 / 3.f
	float FaceRegion;           //threshold of perfect portrait height, default:0.22f
	int Food_Threslow;          //lower threshold of food ratio, default:95
	int Food_Threshigh;         //upper threshold of food ratio, default:105
	float Food_ThresGood;       //"Good" threshold of food guide, default:0.08f
	float Food_ThresPerfect;    //"Perfect" threshold of food guide, default:0.04f
	int Line_ThresAngle1;       //"Good" threshold of line angle, default:6
	int Line_ThresAngle2;       //"Perfect" threshold of line angle, default:3
	float Line_ThresDisratio;   //threshold of distance between horizon line and detected line

	int compose_enable_flag;    //enable flag for composition
};

} /* namespace smart */
#endif /* SMART_KIT_SMART_COMPOSITION_H_ */
