/*
 *  smart_planet.h
 *
 *  Created on: 2016/07/11
 *      Author: xufeng8
 *
 * 	it's a scalable, flexible platform for intelligent modules
 *
 *  Revised on:
 *      Author:
 */

#include "smart_extern_manager.h"
#include "smart_type.h"


namespace smart{

	class SmartPlanet{
	public:
		SmartPlanet();
		virtual ~SmartPlanet();

	    /**
	     * do tracking
	     * @param input  input data
	     * @param width  width of input data
	     * @param height height of input data
	     * @return case of process
	     */
		int  smart_input(unsigned char* input, int width, int height);

	    /**
	     * output
	     * @param OutputParam**  address of pointer of OutputParam Structure
	     * @return
	     */
		void smart_output(OutputParam** output);

		/**
		 * set the threshold for tuning composition
		 * @param paramI   int type parameters
		 * @param len1     length of param1
		 * @param paramF   float type parameters
		 * @param len2     length of param2
		 * @return         true:success to tuning|false:fail to tuning
		 */
		bool smart_config_composition(void* paramI, int len1, void* paramF, int len2);

	private:
		int smart_sequence[4];                         	//detection pipeline {1, 2, 3, 4}
		int smart_sequence_index;                     	//index of detection pipeline
		
		bool is_normal_sequence;                       	//detection priority flag
		
		smart::SmartExternManager* extern_manager;      //sensor data manager
		
		OutputParam output_container;					//bearing the output result from smart_planet
		
		/**
		 * obtain the result from smart_composition
		 * @param flag  type of scenes
		 * @return   1 :success to tuning| 2 :fail to tuning
		 */
		int  smart_output_composition(int flag);

		/**
		 * obtain the result from smart_recognition
		 * @param
		 * @return
		 */
		void smart_output_recognition();

		/**
		 * obtain the result from smart_always_in_focus
		 * @param
		 * @return
		 */
		void smart_output_alwaysinfocus();

	};
}
