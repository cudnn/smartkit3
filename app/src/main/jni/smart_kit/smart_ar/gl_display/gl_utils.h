/*
 *  gl_utils.h
 *
 *  this file is part of smart_ar
 *
 *  Copyright lenovo
 *
 *  Created on: 2015-11-24
 *  Author(s): MikeyNa
 */
#ifndef GL_UTILS_H_HH
#define GL_UTILS_H_HH

#include <jni.h>
#include <android/log.h>

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

void check_gl_error(const char* op);
GLuint bind_texture(GLuint texture, const char *buffer, GLuint w , GLuint h, GLenum internalforma);
GLuint build_shader(const char* source, GLenum shader_type);
GLuint build_program(const char* vertex_shader_source,
		const char* fragment_shader_source);
#endif
