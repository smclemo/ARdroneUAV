/*
 *  video_stage.c
 *  Test
 *
 *  Created by Frédéric D'HAEYER on 22/02/10.
 *  Copyright 2010 Parrot SA. All rights reserved.
 *
 */
#include <VP_Api/vp_api.h>
#include <VP_Api/vp_api_error.h>
#include <VP_Api/vp_api_stage.h>
#include <VP_Api/vp_api_picture.h>

#include <VLIB/Stages/vlib_stage_decode.h>
#include <VP_Stages/vp_stages_yuv2rgb.h>
#include <ardrone_tool/ardrone_tool.h>
#include <ardrone_tool/Com/config_com.h>
#include <ardrone_tool/Video/video_com_stage.h>
//#define RECORD_VIDEO
#ifdef RECORD_VIDEO
#    include <ardrone_tool/Video/video_stage_recorder.h>
#endif

#include "video_stage.h"
#include "opengl_stage.h"

#define NB_STAGES 5

PIPELINE_HANDLE pipeline_handle;
static bool_t video_stage_in_pause = TRUE;
static vp_os_cond_t video_stage_condition;
static vp_os_mutex_t video_stage_mutex;
static video_com_config_t icc = { 0 };

void video_stage_init(void)
{
	vp_os_mutex_init(&video_stage_mutex);
	vp_os_cond_init(&video_stage_condition, &video_stage_mutex);
}

void video_stage_suspend_thread(void)
{
	vp_os_mutex_lock(&video_stage_mutex);
	video_stage_in_pause = TRUE;
	vp_os_mutex_unlock(&video_stage_mutex);	
}

void video_stage_resume_thread(void)
{
	vp_os_mutex_lock(&video_stage_mutex);
	vp_os_cond_signal(&video_stage_condition);
	video_stage_in_pause = FALSE;
	vp_os_mutex_unlock(&video_stage_mutex);	
}

DEFINE_THREAD_ROUTINE(video_stage, data)
{
	C_RESULT res;
	
	vp_api_io_pipeline_t    pipeline;
	vp_api_io_data_t        out;
	vp_api_io_stage_t       stages[NB_STAGES];
	
	vp_api_picture_t picture;
	
	vlib_stage_decoding_config_t    vec;

	video_stage_in_pause = FALSE;
	
	vp_os_memset(&icc,          0, sizeof( icc ));
	vp_os_memset(&vec,          0, sizeof( vec ));
	vp_os_memset(&picture,      0, sizeof( picture ));

#ifdef RECORD_VIDEO
	video_stage_recorder_config_t vrc;
#endif
	
	/// Picture configuration
	picture.format        = PIX_FMT_RGB565;
	
	picture.width         = 512;
	picture.height        = 512;
	picture.framerate     = 15;
	
	picture.y_buf   = vp_os_malloc( picture.width * picture.height * 2);
	picture.cr_buf  = NULL;
	picture.cb_buf  = NULL;
	
	picture.y_line_size   = picture.width * 2;
	picture.cb_line_size  = 0;
	picture.cr_line_size  = 0;
		
	icc.com                 = COM_VIDEO();
	icc.buffer_size         = 100000;
	icc.protocol            = VP_COM_UDP;
	COM_CONFIG_SOCKET_VIDEO(&icc.socket, VP_COM_CLIENT, VIDEO_PORT, wifi_ardrone_ip);
	
	vec.width               = 512;
	vec.height              = 512;
	vec.picture             = &picture;
	vec.luma_only           = FALSE;
	vec.block_mode_enable   = TRUE;
	
	pipeline.nb_stages = 0;
	
	stages[pipeline.nb_stages].type    = VP_API_INPUT_SOCKET;
	stages[pipeline.nb_stages].cfg     = (void *)&icc;
	stages[pipeline.nb_stages].funcs   = video_com_funcs;
	pipeline.nb_stages++;
	
	stages[pipeline.nb_stages].type    = VP_API_FILTER_DECODER;
	stages[pipeline.nb_stages].cfg     = (void*)&vec;
	stages[pipeline.nb_stages].funcs   = vlib_decoding_funcs;
	pipeline.nb_stages++;
	
#ifdef RECORD_VIDEO
	stages[pipeline.nb_stages].type    = VP_API_FILTER_DECODER;
	stages[pipeline.nb_stages].cfg     = (void*)&vrc;
	stages[pipeline.nb_stages].funcs   = video_recorder_funcs;
	pipeline.nb_stages++;
#endif

	stages[pipeline.nb_stages].type    = VP_API_OUTPUT_LCD;
	stages[pipeline.nb_stages].cfg     = (void*)&vec;
	stages[pipeline.nb_stages].funcs   = opengl_video_stage_funcs;
	pipeline.nb_stages++;
		
	pipeline.stages = &stages[0];
		
	if( !ardrone_tool_exit() )
	{
		PRINT("\n   video stage thread initialisation\n\n");
		
		res = vp_api_open(&pipeline, &pipeline_handle);
		
		if( SUCCEED(res) )
		{
			int loop = SUCCESS;
			out.status = VP_API_STATUS_PROCESSING;
#ifdef RECORD_VIDEO		    
			{
				DEST_HANDLE dest;
				dest.stage = 2;
				dest.pipeline = pipeline_handle;
				vp_api_post_message( dest, PIPELINE_MSG_START, NULL, (void*)NULL);
			}
#endif			
			
			while( !ardrone_tool_exit() && (loop == SUCCESS) )
			{
				if(video_stage_in_pause)
				{
					vp_os_mutex_lock(&video_stage_mutex);
					icc.num_retries = VIDEO_MAX_RETRIES;
					vp_os_cond_wait(&video_stage_condition);
					vp_os_mutex_unlock(&video_stage_mutex);
				}
				
				if( SUCCEED(vp_api_run(&pipeline, &out)) ) {
					if( (out.status == VP_API_STATUS_PROCESSING || out.status == VP_API_STATUS_STILL_RUNNING) ) {
						loop = SUCCESS;
					}
				}
				else loop = -1; // Finish this thread
			}
			
			vp_api_close(&pipeline, &pipeline_handle);
		}
	}
	
	PRINT("   video stage thread ended\n\n");
	
	return (THREAD_RET)0;
}

uint32_t video_stage_get_num_retries(void)
{
	return icc.num_retries;
}
