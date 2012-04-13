
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#ifdef USE_BLUES32
# warning USE_BLUES32 must be undefined. It is turned to undefined.
# undef USE_BLUES32
#endif

#include <config.h>
#include <ATcodec/ATcodec_api.h>
#include <VP_Os/vp_os_types.h>
#include <VP_Com/vp_com.h>
#include <VP_Com/vp_com_serial.h>
#include <VP_Api/vp_api_error.h>
#include <VP_Os/vp_os_signal.h>
#include <VP_Os/vp_os_delay.h>
#include <VP_Os/vp_os_thread.h>
#include <VP_Os/vp_os_print.h>
#include <VP_Os/vp_os_malloc.h>
#include <VP_Stages/vp_stages_io_com.h>

AT_CODEC_MSG_ID AT_MSG_ATCMD_DEFAULT;

#include <at_msgs_ids.h>
#include "at_msgs_impl.h"
#include <Com/config_com.h>

#include <common/mobile_config.h>

#define ATCMD_INPUT_SIZE 1024

AT_CODEC_MSG_IDS ids;

static vp_stages_output_com_config_t occ;

static Write atcodec_write = NULL;
static Read atcodec_read = NULL;

#ifdef USE_SERIAL
  static vp_com_serial_config_t    config;
#endif

AT_CODEC_ERROR_CODE at_mgmt_init(void)
{
# undef ATCODEC_DEFINE_AT_CMD
# define ATCODEC_DEFINE_AT_CMD(ID,Str,From,Cb,Prio) \
    if((ids.ID = ATcodec_Add_Defined_Message(Str)) == -1) \
      { \
        fprintf(stderr, "Error Add_Hashed \"%s\" library\n", Str); \
        return AT_CODEC_INIT_ERROR; \
      }

# undef ATCODEC_DEFINE_AT_RESU
# define ATCODEC_DEFINE_AT_RESU(ID,Str,From,Cb) \
    if((ids.ID = ATcodec_Add_Hashed_Message(Str,ids.From,Cb,0)) == -1) \
      { \
        fprintf(stderr, "Error Add_Defined \"%s\" library\n", Str); \
        return AT_CODEC_INIT_ERROR; \
      }

# include AT_MESSAGES_HEADER

  return AT_CODEC_INIT_OK;
}

AT_CODEC_ERROR_CODE at_mgmt_shutdown(void)
{
  ATcodec_Shutdown_Library();

  return AT_CODEC_SHUTDOWN_OK;
}

AT_CODEC_ERROR_CODE at_mgmt_enable(void)
{
  return AT_CODEC_ENABLE_OK;
}

AT_CODEC_ERROR_CODE at_mgmt_open(void)
{
  static bool_t init_ok = FALSE;

  if( !init_ok )
  {
    vp_os_memset( &occ, 0, sizeof(vp_stages_output_com_config_t) );

    occ.com                   = COM_AT();
    occ.config                = COM_CONFIG_AT();
    occ.connection            = COM_CONNECTION_AT();
    occ.buffer_size           = ATCMD_INPUT_SIZE;

    COM_CONFIG_SOCKET_AT(&occ.socket, VP_COM_CLIENT, AT_PORT, WIFI_ARDRONE_IP);

    if(FAILED(vp_com_init(occ.com)))
    {
      DEBUG_PRINT_SDK("VP_Com : Failed to init com for atcmd\n");
      vp_com_shutdown(occ.com);
      return AT_CODEC_OPEN_ERROR;
    }

    if(FAILED(vp_com_local_config(occ.com, occ.config)))
    {
      DEBUG_PRINT_SDK("VP_Com : Failed to configure com for atcmd\n");
      vp_com_shutdown(occ.com);
      return AT_CODEC_OPEN_ERROR;
    }

    if(FAILED(vp_com_connect(occ.com, occ.connection, 1)))
    {
      DEBUG_PRINT_SDK("VP_Com : Failed to connect for atcmd\n");
      return AT_CODEC_OPEN_ERROR;
    }

    DEBUG_PRINT_SDK("com OK for atcmd\n");

    DEBUG_PRINT_SDK("ATcodec_open\n");

    if(FAILED(vp_com_open(occ.com, &occ.socket, &atcodec_read, &atcodec_write)))
    {
      DEBUG_PRINT_SDK("VP_Com: open failed for ATcodec\n");
      return AT_CODEC_OPEN_ERROR;
    }

    init_ok = TRUE;
  }

  return AT_CODEC_OPEN_OK;
}

AT_CODEC_ERROR_CODE at_mgmt_close(void)
{
  DEBUG_PRINT_SDK("ATcodec_close\n");

  vp_com_close(occ.com, &occ.socket);

  DEBUG_PRINT_SDK("socket closed\n");

  return AT_CODEC_CLOSE_OK;
}

AT_CODEC_ERROR_CODE at_mgmt_write(int8_t *buffer, int32_t *len)
{
  if( atcodec_write != NULL )
  {
    //DEBUG_PRINT_SDK("ATcodec_write\n");
    return FAILED(atcodec_write(&occ.socket, buffer, len)) ? AT_CODEC_WRITE_ERROR : AT_CODEC_WRITE_OK;
  }

  return AT_CODEC_WRITE_OK;
}

AT_CODEC_ERROR_CODE at_mgmt_read(int8_t *buffer, int32_t *len)
{
  //  DEBUG_PRINT_SDK("ATcodec_read\n");
/*   return FAILED(atcodec_read(&occ.socket, buffer, len)) ? AT_CODEC_READ_ERROR : AT_CODEC_READ_OK; */
  return AT_CODEC_READ_OK;
}

