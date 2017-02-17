/*
 *  forground_renderer.h
 *
 *  this file is part of smart_ar
 *  class ForgroundRenderer
 *
 *  Copyright lenovo
 *
 *  Created on: 2015-11-24
 *  Author(s): MikeyNa
 */
#define _ANDROID_
#define DEBUG

#include <jni.h>
#include <android/log.h>

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "log.h"

class ForgroundRenderer {
public:
	ForgroundRenderer();
	~ForgroundRenderer();
	void init();
	void uninitialize();
	bool render();
private:
	void draw();

private:
	int ATTRIB_VERTEX, ATTRIB_TEXTURE;

	GLuint _texture_id;
	GLuint _program;
	char * _data_buffer;
	int _width;
	int _height;
	const char * _vertShaderString;
	const char * _fragShaderString;
};
