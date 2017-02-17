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
#include "forground_renderer.h"

#include <jni.h>
#include <android/log.h>

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "log.h"
#include "gl_utils.h"

ForgroundRenderer::ForgroundRenderer() {

	_data_buffer = NULL;
	_fragShaderString =
	    "#ifdef GL_ES\n"
	    "precision mediump float;\n"
	    "#endif\n"
	    "varying highp vec2 texCoordVarying;\n"
	    "uniform sampler2D texture3;\n"
	    "void main()\n"
	    "{\n"
			"float gray = 0.0;"
	        "gray  = texture2D(texture3, texCoordVarying).r;\n"
	        "gl_FragColor = vec4(gray, gray,  gray, 1.0);\n"
	    "}\n";
	_vertShaderString =
	    "attribute vec4 vPosition;\n"
	    "attribute vec2 a_texCoord;\n"
	    "varying vec2 texCoordVarying;\n"
	    "void main()\n"
	    "{\n"
	        "gl_Position = vPosition;\n"
	        "texCoordVarying = a_texCoord;\n"
	    "}\n";
}
ForgroundRenderer::~ForgroundRenderer()
{
	uninitialize();
}

void ForgroundRenderer::draw()
{
    static GLfloat squareVertices[] = {
        -0.5f, -0.5f,
        0.5f, -0.5f,
        -0.5f,  0.5f,
        0.5f,  0.5f,
    };

    static GLfloat coordVertices[] = {
        0.0f, 1.0f,
        1.0f, 1.0f,
        0.0f,  0.0f,
        1.0f,  0.0f,
    };

	GLint tex_y = glGetUniformLocation(_program, "texture3");
    check_gl_error("glGetUniformLocation");

    ATTRIB_VERTEX=glGetAttribLocation(_program,"vPosition");
    check_gl_error("glBindAttribLocation");

    ATTRIB_TEXTURE=glGetAttribLocation(_program,"a_texCoord");
    check_gl_error("glBindAttribLocation");

    glVertexAttribPointer(ATTRIB_VERTEX, 2, GL_FLOAT, 0, 0, squareVertices);
    check_gl_error("glVertexAttribPointer");
    glEnableVertexAttribArray(ATTRIB_VERTEX);
    check_gl_error("glEnableVertexAttribArray");

    glVertexAttribPointer(ATTRIB_TEXTURE, 2, GL_FLOAT, 0, 0, coordVertices);
    check_gl_error("glVertexAttribPointer");
    glEnableVertexAttribArray(ATTRIB_TEXTURE);
    check_gl_error("glEnableVertexAttribArray");

	glActiveTexture(GL_TEXTURE4);
	check_gl_error("glActiveTexture");
	glBindTexture(GL_TEXTURE_2D, _texture_id);
	check_gl_error("glBindTexture");
	glUniform1i(tex_y, 4);
	check_gl_error("glUniform1i");


	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	check_gl_error("glDrawArrays");
}

void ForgroundRenderer::init()
{
	_program = build_program(_vertShaderString, _fragShaderString);
    glGenTextures(1, &_texture_id);
}

void ForgroundRenderer::uninitialize()
{
    if (_data_buffer)
    {
        free(_data_buffer);
        _data_buffer = NULL;
    }
}

bool ForgroundRenderer::render()
{
    int width = 1280;
    int height = 720;

    if (!_data_buffer)
    {
    	int YUVLENGTH = 720*1280*3/2;
    	_data_buffer = (char*)malloc(YUVLENGTH);
    	FILE *p = fopen("/sdcard/a.yuv", "rb");
    	fread(_data_buffer, sizeof(unsigned char), YUVLENGTH, p);
    	fclose(p);
    }
    if (!_data_buffer)
        return false;

    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);

    glUseProgram(_program);
    check_gl_error("glUseProgram");
    bind_texture(_texture_id, _data_buffer, width, height, GL_LUMINANCE);
    draw();

    glDisable(GL_BLEND);

    glFinish();
    return true;
}
