/*
 *  opengl_stage.h
 *  Test
 *
 *  Created by Karl Leplat on 22/02/10.
 *  Copyright 2010 Parrot SA. All rights reserved.
 *
 */
#ifndef _OPENGL_STAGE_H_
#define _OPENGL_STAGE_H_
#include "ConstantsAndMacros.h"

typedef struct _opengl_video_config_t
{
	vp_os_mutex_t mutex;
	GLuint screenWidth;
	GLuint screenHeight;
	GLuint widthImage;
	GLuint heightImage;
	GLuint widthTexture;
	GLuint heightTexture;
	
	GLfloat scaleModelX;
	GLfloat scaleModelY;
	GLfloat scaleTextureX;
	GLfloat scaleTextureY;
	GLuint bytesPerPixel;
	GLenum format;
	GLenum type;
	void* data;
	GLuint identifier;
	uint32_t num_picture_decoded;
	
	uint16_t old_width, old_height;
} opengl_video_stage_config_t;

C_RESULT opengl_video_stage_open(vlib_stage_decoding_config_t *cfg);
C_RESULT opengl_video_stage_transform(vlib_stage_decoding_config_t *cfg, vp_api_io_data_t *in, vp_api_io_data_t *out);
C_RESULT opengl_video_stage_close(vlib_stage_decoding_config_t *cfg);

opengl_video_stage_config_t* opengl_video_stage_get(void);

extern const vp_api_stage_funcs_t opengl_video_stage_funcs;
extern const opengl_video_stage_config_t* opengl_video_stage_config;

#endif // _OPENGL_STAGE_H_