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

#include "background_renderer.h"

static const char vertShaderString[] = "attribute vec4 vPosition;\n"
		"attribute vec2 a_texCoord;\n"
		"varying vec2 texCoordVarying;\n"
		"void main()\n"
		"{\n"
		"gl_Position = vPosition;\n"
		"texCoordVarying = a_texCoord;\n"
		"}\n";
static const char fragShaderStringYCrCbITURec601FullRangeBiPlanar[] =
		"#ifdef GL_ES\n"
				"precision mediump float;\n"
				"#endif\n"
				"varying highp vec2 texCoordVarying;\n"
				"uniform sampler2D texture0;\n"
				"uniform sampler2D texture1;\n"
				"void main()\n"
				"{\n"
				"vec3 colourYCrCb;\n"
				"const mat3 transformYCrCbITURec601FullRangeToRGB = mat3(1.0,    1.0,   1.0,\n" // Column 0
				"1.402, -0.714, 0.0,\n"// Column 1
				"0.0,   -0.344, 1.772);\n"// Column 2
				"colourYCrCb.x  = texture2D(texture0, texCoordVarying).r;\n"
				"colourYCrCb.yz = texture2D(texture1, texCoordVarying).ra - 0.5;\n"
				"gl_FragColor = vec4(transformYCrCbITURec601FullRangeToRGB * colourYCrCb, 1.0);\n"
				"}\n";

BackgroundRenderer::BackgroundRenderer() {

	_data_buffer = NULL;
	_width = 0;
	_height = 0;
}
BackgroundRenderer::~BackgroundRenderer()
{
	uninitialize();
}
void BackgroundRenderer::draw() {
	static GLfloat squareVertices[] = { -1.0f, -1.0f, 1.0f, -1.0f, -1.0f, 1.0f,
			1.0f, 1.0f, };
#if 1//camera 2
	static GLfloat coordVertices[] = { 0.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
			0.0f, };
#else
	static GLfloat coordVertices[] = {
			1.0f,  1.0f,
	        1.0f,  0.0f,
	        0.0f, 1.0f,
	        0.0f, 0.0f };
#endif


//	glClearColor(1.0f, 1.0f, 1.0f, 1);
//	check_gl_error("glClearColor");
//	glClear (GL_COLOR_BUFFER_BIT);
//	check_gl_error("glClear");
	GLint tex_y = glGetUniformLocation(_program, "texture0");
	check_gl_error("glGetUniformLocation");
	GLint tex_u = glGetUniformLocation(_program, "texture1");
	check_gl_error("glGetUniformLocation");

	ATTRIB_VERTEX = glGetAttribLocation(_program, "vPosition");
	check_gl_error("glBindAttribLocation");

	ATTRIB_TEXTURE = glGetAttribLocation(_program, "a_texCoord");
	check_gl_error("glBindAttribLocation");

	glVertexAttribPointer(ATTRIB_VERTEX, 2, GL_FLOAT, 0, 0, squareVertices);
	check_gl_error("glVertexAttribPointer");
	glEnableVertexAttribArray(ATTRIB_VERTEX);
	check_gl_error("glEnableVertexAttribArray");

	glVertexAttribPointer(ATTRIB_TEXTURE, 2, GL_FLOAT, 0, 0, coordVertices);
	check_gl_error("glVertexAttribPointer");
	glEnableVertexAttribArray(ATTRIB_TEXTURE);
	check_gl_error("glEnableVertexAttribArray");

	glActiveTexture (GL_TEXTURE0);
	check_gl_error("glActiveTexture");
	glBindTexture(GL_TEXTURE_2D, _tex_y_id);
	check_gl_error("glBindTexture");
	glUniform1i(tex_y, 0);
	check_gl_error("glUniform1i");

	glActiveTexture (GL_TEXTURE1);
	check_gl_error("glActiveTexture");
	glBindTexture(GL_TEXTURE_2D, _tex_uv_id);
	check_gl_error("glBindTexture");
	glUniform1i(tex_u, 1);
	check_gl_error("glUniform1i");

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	check_gl_error("glDrawArrays");
}
void BackgroundRenderer::init() {
	_data_buffer = NULL;

	_program = build_program(vertShaderString,
			fragShaderStringYCrCbITURec601FullRangeBiPlanar);
	glGenTextures(1, &_tex_y_id);
	glGenTextures(1, &_tex_uv_id);
}

void BackgroundRenderer::uninitialize() {
	if (_data_buffer != NULL) {
		free(_data_buffer);
		_data_buffer = NULL;
	}
}
void BackgroundRenderer::set_framebuffer(const char* buffer, int buffersize,
		int width, int height) {
	SMART_DEBUG("ARGLrender::Line = %d\n", __LINE__);
	if (_width != width || _height != height) {
		if (_data_buffer != NULL)
			free(_data_buffer);

		_width = width;
		_height = height;

		_data_buffer = (char *) malloc(buffersize);
	}

	if (_data_buffer)
		memcpy(_data_buffer, buffer, buffersize);
}

bool BackgroundRenderer::render() {
	if(_data_buffer == NULL)
		return false;
	const char *buffer = _data_buffer;
	int width = _width;
	int height = _height;
	SMART_DEBUG("ARGLrender::Line = %d\n", __LINE__);
	bind_texture(_tex_y_id, buffer, width, height, GL_LUMINANCE);
	SMART_DEBUG("ARGLrender::Line = %d\n", __LINE__);
	bind_texture(_tex_uv_id, buffer + width * height, width / 2, height / 2,
			GL_LUMINANCE_ALPHA);
	SMART_DEBUG("ARGLrender::Line = %d\n", __LINE__);
	glUseProgram(_program);
	SMART_DEBUG("ARGLrender::Line = %d\n", __LINE__);
	draw();
	SMART_DEBUG("ARGLrender::Line = %d\n", __LINE__);
	glFinish();
	SMART_DEBUG("ARGLrender::Line = %d\n", __LINE__);
	return true;
}
