#ifndef _ARDRONE_NAVDATA_CLIENT_H_
#define _ARDRONE_NAVDATA_CLIENT_H_

#include <VP_Os/vp_os_types.h>
#include <VP_Api/vp_api_thread_helper.h>

#include <ardrone_api.h>
#include <ardrone_tool/Control/ardrone_navdata_control.h>
#include <ardrone_tool/Navdata/ardrone_navdata_file.h>
#include <ardrone_tool/Navdata/ardrone_general_navdata.h>
#include <ardrone_tool/Navdata/ardrone_academy_navdata.h>
#include <ardrone_tool/Video/video_navdata_handler.h>
#include <config.h>

#define NAVDATA_MAX_RETRIES     5


typedef enum
{
    NAVDATA_BOOTSTRAP = 0,
    NAVDATA_DEMO,
    NAVDATA_FULL
} navdata_mode_t;


uint32_t ardrone_navdata_client_get_num_retries(void);
C_RESULT ardrone_navdata_client_init(void);
C_RESULT ardrone_navdata_client_suspend(void);
C_RESULT ardrone_navdata_client_resume(void);
C_RESULT ardrone_navdata_client_shutdown(void);
C_RESULT ardrone_navdata_open_server(void);

#endif // _ARDRONE_NAVDATA_H_
