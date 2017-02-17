/*
 *  background_renderer.h
 *
 *  this file is part of smart_ar
 *  class BackgroundRenderer
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
#include "gl_utils.h"

class BackgroundRenderer {
public:
	BackgroundRenderer();
	~BackgroundRenderer();
	void init();
	void uninitialize();
	void set_framebuffer(const char* buffer, int buffersize, int width,
			int height);
	bool render();

private:
	void draw();
	int ATTRIB_VERTEX, ATTRIB_TEXTURE;
	GLuint _tex_y_id;
	GLuint _tex_uv_id;
	GLuint _program;
	int _width;
	int _height;
	char * _data_buffer;
};
