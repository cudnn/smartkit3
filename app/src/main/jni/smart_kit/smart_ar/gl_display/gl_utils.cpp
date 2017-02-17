/*
 *  gl_utils.cpp
 *
 *  this file is part of smart_ar
 *
 *  Copyright lenovo
 *
 *  Created on: 2015-11-24
 *  Author(s): MikeyNa
 */
#include <jni.h>
#include <android/log.h>

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "log.h"

void check_gl_error(const char* op)
{
    GLint error;
    for (error = glGetError(); error; error = glGetError())
    {
    	SMART_DEBUG("error::after %s() glError (0x%x)\n", op, error);
    }
}

 GLuint bind_texture(GLuint texture, const char *buffer, GLuint w , GLuint h, GLenum internalforma)
{
	check_gl_error("glGenTextures");
	glBindTexture ( GL_TEXTURE_2D, texture );
	check_gl_error("glBindTexture");
	glTexImage2D ( GL_TEXTURE_2D, 0, internalforma, w, h, 0, internalforma, GL_UNSIGNED_BYTE, buffer);
	check_gl_error("glTexImage2D");
	glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	check_gl_error("glTexParameteri");
	glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	check_gl_error("glTexParameteri");
	glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
	check_gl_error("glTexParameteri");
	glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
	check_gl_error("glTexParameteri");

	return texture;
}


GLuint build_shader(const char* source, GLenum shader_type)
{
	GLuint shader_handle = glCreateShader(shader_type);
    if (shader_handle)
    {
	    glShaderSource(shader_handle, 1, &source, 0);
	    glCompileShader(shader_handle);

        GLint compiled = 0;
        glGetShaderiv(shader_handle, GL_COMPILE_STATUS, &compiled);
        if (!compiled)
        {
            GLint infoLen = 0;
            glGetShaderiv(shader_handle, GL_INFO_LOG_LENGTH, &infoLen);
            if (infoLen)
            {
                char* buf = (char*) malloc(infoLen);
                if (buf)
                {
                    glGetShaderInfoLog(shader_handle, infoLen, NULL, buf);
                    SMART_DEBUG("error::Could not compile shader %d:\n%s\n", shaderType, buf);
                    free(buf);
                }
                glDeleteShader(shader_handle);
                shader_handle = 0;
            }
        }

    }

    return shader_handle;
}

GLuint build_program(const char* vertex_shader_source,
		const char* fragment_shader_source)
{
	GLuint vertex_shader = build_shader(vertex_shader_source, GL_VERTEX_SHADER);
	GLuint fragment_shader = build_shader(fragment_shader_source, GL_FRAGMENT_SHADER);
	GLuint program_handle = glCreateProgram();
    if (program_handle)
    {
	    glAttachShader(program_handle, vertex_shader);
	    check_gl_error("glAttachShader");
	    glAttachShader(program_handle, fragment_shader);
	    check_gl_error("glAttachShader");
	    glLinkProgram(program_handle);

        GLint link_status = GL_FALSE;
        glGetProgramiv(program_handle, GL_LINK_STATUS, &link_status);
        if (link_status != GL_TRUE) {
            GLint buf_length = 0;
            glGetProgramiv(program_handle, GL_INFO_LOG_LENGTH, &buf_length);
            if (buf_length) {
                char* buf = (char*) malloc(buf_length);
                if (buf) {
                    glGetProgramInfoLog(program_handle, buf_length, NULL, buf);
                    SMART_DEBUG("error::Could not link program:\n%s\n", buf);
                    free(buf);
                }
            }
            glDeleteProgram(program_handle);
            program_handle = 0;
        }

    }
	return program_handle;
}
