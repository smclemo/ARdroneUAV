/**
 *  \brief    gamepad handling implementation
 *  \author   Sylvain Gaeremynck <sylvain.gaeremynck@parrot.fr>
 *  \version  1.0
 *  \date     04/06/2007
 *  \warning  Subject to completion
 */


#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <stddef.h>
#include <stdio.h>
#include <unistd.h>

#include <linux/joystick.h>

#include "gamepad.h"
extern "C" {
#include <ATcodec/ATcodec_api.h>
#include <VP_Os/vp_os_print.h>
#include <ardrone_api.h>
}

#include "planner.hpp"

typedef struct {
  int32_t bus;
  int32_t vendor;
  int32_t product;
  int32_t version;
  char    name[MAX_NAME_LENGTH];
  char    handlers[MAX_NAME_LENGTH];
} device_t;

input_device_t gamepad = {
  "Gamepad",
  open_gamepad,
  update_gamepad,
  close_gamepad
};

static int32_t joy_dev = 0;
FILE *commandLog = NULL;

int32_t lastx, lasty, lastz, lastyaw;
//Function wrappers to allow C calls in C++:
extern "C" {
  int ARDRONE_STARTED = 0;
  int repeatCount = 0;
  void ardrone_at_set_led_animationC ( /*LED_ANIMATION_IDS*/int anim_id, float32_t freq, uint32_t duration_sec)
    { printf("LED ANIMATION NOT IMPLEMENTED\n"); }
  void ardrone_at_zapC(ZAP_VIDEO_CHANNEL channel){ardrone_at_zap(channel);}
  void start_stop()
  {
    ARDRONE_STARTED = ARDRONE_STARTED ? 0 : 1;
    ardrone_tool_set_ui_pad_start(ARDRONE_STARTED);
	}
  int ardrone_at_set_radiogp_inputC(int32_t y, int32_t x, int32_t z, int32_t yaw){
    
    if(y == lasty && x == lastx && z == lastz && yaw == lastyaw && repeatCount < 10) {
      repeatCount++;
      return 0; //only continue if does not mach last command sent
    }
    repeatCount = 0;
    /*if(commandLog == NULL)
    {
      char filename[256];
      sprintf(filename, "ControlLog-%d.txt", (int)time(NULL));
      commandLog = fopen(filename, "w");
      fprintf(commandLog, "Time Pitch Roll Gaz Yaw\n");
    }
    else
    {
      timeval t;
      gettimeofday(&t, NULL);
      fprintf(commandLog, "%d.%06d %d %d %d %d \n", (int)t.tv_sec, (int)t.tv_usec, y, x, z, yaw);
    }*/
    
    /************* at_set_radiogp_input ****************
    * Description : Fill struct radiogp_cmd, 
    * used with at_cmds_loop function.
    * pitch : y-axis (rad) (-25000, +25000)  
    * roll : x-axis (rad) (-25000, +25000) 
    * gaz :  altitude (mm/s) (-25000, +25000)
    * yaw : z-axis (rad/s) (-25000, +25000)
    */
      //Make sure commands are in range
      if(x < -25000) x = -25000;
      if(x > 25000) x = 25000;
      if(y < -25000) y = -25000;
      if(y > 25000) y = 25000;
      if(z < -25000) z = -25000;
      if(z > 25000) z = 25000;
      if(yaw < -25000) yaw = -25000;
      if(yaw > 25000) yaw = 25000;
      ardrone_at_set_progress_cmd( 1,
                                   x/25000.0,
                                   y/25000.0,
                                   -z/25000.0,
                                   yaw/25000.0);
      //ardrone_at_set_radiogp_input(y, x, z, yaw); old command
      PRINT("Pitch:%3d Roll:%3d Gaz:%3d Yaw:%3d\n", y, x, z, yaw);
      lastx = x;
      lasty = y;
      lastz = z;
      lastyaw = yaw;
      return 1;
  }

  /*void *Control_Thread(void *params)
  {
    while(1)
    {
      ardrone_at_set_radiogp_input(lasty, lastx, lastz, lastyaw);
      PRINT("Pitch:%3d Roll:%3d Gaz:%3d Yaw:%3d\n", lasty, lastx, lastz, lastyaw);
      usleep(50000);
    }
  }*/
}

///////////////////////////////
//  GamePad input functions  //
///////////////////////////////
/*extern*/ Planner *planner;

extern int CAPTUREIMAGESTREAM;

C_RESULT open_gamepad(void)
{
  C_RESULT res = C_FAIL;
  
  //pthread_t thread;
  //pthread_create( &thread, NULL, Control_Thread, (void*) NULL);

  //FILE* f = fopen("/proc/bus/input/devices", "r");

//  if( f != NULL )
//  {
    if(!(GAMEPAD_OVERRIDE))
      //res = parse_proc_input_devices( f, GAMEPAD_LOGICTECH_ID);
      res = C_FAIL;
    else
    {
      strcpy(gamepad.name, GAMEPAD_OS_ID);
      //res = add_device(GAMEPAD_LOGICTECH_ID);
      res = C_OK;
    }
//    fclose( f );

    if( SUCCEED( res ) && strcmp(gamepad.name, "Gamepad")!=0)
    {
			char dev_path[20]="/dev/input/";
			strcat(dev_path, gamepad.name);
      joy_dev = open(dev_path, O_NONBLOCK | O_RDONLY);
    }
		else
		{
			return C_FAIL;
		}
//  }

  return res;
}

#define xfactor 1.4
#define yfactor 1.4
#define yawfactor 1.4
#define zfactor 1.4
static int32_t x = 0, y = 0, yaw = 0, z = 0;
C_RESULT update_gamepad(void)
{
  static bool_t refresh_values = FALSE;
  ssize_t res;
  static struct js_event js_e_buffer[64];

  //If algorithms are enabled, update them before checking joystick:
  //*****ALGORITHM CONTROL*****
  /*if(planner->enabled)
  {
    y = planner->dpitch_final;
    x = planner->droll_final;
    yaw = planner->dyaw_final;
    z = planner->dgaz_final;
    ardrone_at_set_radiogp_inputC(y, x, z, yaw); //range checking done in C wrapper
    PRINT("Pitch:%3d Roll:%3d Gaz:%3d Yaw:%3d\n", y, x, z, yaw);
  }
  //Handled in keyboard.cpp
  */
  
  res = read(joy_dev, js_e_buffer, sizeof(struct js_event) * 64);
  
  if( !res || (res < 0 && errno == EAGAIN) )
    return C_OK;

  if( res < 0 )
    return C_OK;

  if (res < (int) sizeof(struct js_event))// If non-complete bloc: ignored
    return C_OK;
  
  // Buffer decomposition in blocs (if the last is incomplete, it's ignored)
  uint32_t idx = 0;
  refresh_values = FALSE;
  for (idx = 0; idx < res / sizeof(struct js_event); idx++)
  {
    if(js_e_buffer[idx].type & JS_EVENT_INIT )// If Init, the first values are ignored
    {
      break;
    }
    else if(js_e_buffer[idx].type & JS_EVENT_BUTTON )// Event Button detected
    {
      //*****JOYSTICK COMMANDS*****
      switch( js_e_buffer[idx].number )
      {
        case 5 : //5th button (360 RB) to toggle image stream capture
	  if(js_e_buffer[idx].value == 1) 
	    {
	      CAPTUREIMAGESTREAM = CAPTUREIMAGESTREAM ? false : true;
	      if(CAPTUREIMAGESTREAM)
	      {
		  printf("*******************************************\n");
		  printf("**    Starting Image Stream Capture...   **\n");
		  printf("*******************************************\n");
	      }
	      else
	      {
		  printf("*******************************************\n");
		  printf("**    Stopping Image Stream Capture...   **\n");
		  printf("*******************************************\n");
	      }
	    }
          break;

        case 3 : //3rd button (360 Y-Button) to dump image
	  if(js_e_buffer[idx].value == 1) 
	  {
	    char filename[256];
	    timeval t;
	    gettimeofday(&t, NULL);
	    sprintf(filename, "%d.%06d.bmp", (int)t.tv_sec, (int)t.tv_usec);
	    if(frontImgStream != NULL && cvSaveImage(filename, cvCloneImage(frontImgStream)))
	      printf("Front Image dumped to %s\n", filename);
	    else
	      printf("Error dumping image.\n");
	    sprintf(filename, "%d.%06dbot.bmp", (int)t.tv_sec, (int)t.tv_usec);
	    if(bottomImgStream != NULL && cvSaveImage(filename, cvCloneImage(bottomImgStream)))
	      printf("Bottom Image dumped to %s\n", filename);
	    else
	      printf("Error dumping image.\n");
          }
          break;

        case 2 : //2nd button (360 X-Button) for camera toggle
	  if(js_e_buffer[idx].value == 1) ardrone_at_zapC(ZAP_CHANNEL_NEXT);
	  break;
        
        case 1 : //thumb button/360 B-Button for start/stop
          if(js_e_buffer[idx].value == 1) 
          {
            PRINT("******************************************************Starting/Landing...\n");
	    start_stop();
	  }
	  break;

        case 0 : //Joystick trigger/360 A-button enables/disables algorithms
          if(js_e_buffer[idx].value == 1 && planner != NULL)
          {
            planner->enabled = planner->enabled ? FALSE : TRUE;
            if(planner->enabled) PRINT("Algorithms Enabled.\n"); else PRINT("Algorithms Disabled.\n");
            ardrone_at_set_radiogp_inputC(0, 0, 0, 0); //reset
          }
	  break;

        default:
	  break;
      }
    }
    else if(js_e_buffer[idx].type & JS_EVENT_AXIS )// Event Axis detected
    {
      double DeadzoneFix = 1/(1-(JSDEADZONE/25000.0));
      refresh_values = TRUE;
      //******JOYSTICK CONTROL******
      switch( js_e_buffer[idx].number )
      {
        case PAD_X:
          x = (int32_t)((double)( js_e_buffer[idx].value + 1 ) / xfactor);
          if(js_e_buffer[idx].value < JSDEADZONE && js_e_buffer[idx].value > -JSDEADZONE) x = 0;
          else if(js_e_buffer[idx].value > 0) x -= JSDEADZONE / xfactor;
          else x += JSDEADZONE / xfactor;
          x = (double)x * DeadzoneFix;
          break;
        case PAD_Y:
          y = (int32_t)((double)( js_e_buffer[idx].value + 1 ) / yfactor);
          if(js_e_buffer[idx].value < JSDEADZONE && js_e_buffer[idx].value > -JSDEADZONE) y = 0;
          else if(js_e_buffer[idx].value > 0) y -= JSDEADZONE / yfactor;
          else y += JSDEADZONE / yfactor;
          y = (double)y * DeadzoneFix;
          break;
        case PAD_YAW:
          yaw = (int32_t)((double)( js_e_buffer[idx].value + 1 ) / yawfactor);
          if(js_e_buffer[idx].value < JSDEADZONE && js_e_buffer[idx].value > -JSDEADZONE) yaw = 0;
          else if(js_e_buffer[idx].value > 0) yaw -= JSDEADZONE / yawfactor;
          else yaw += JSDEADZONE / yawfactor;
          yaw = (double)yaw * DeadzoneFix;
          break;
        case PAD_Z:
          z = (int32_t)((double)( js_e_buffer[idx].value + 1 ) / zfactor);
          if(js_e_buffer[idx].value < JSDEADZONE && js_e_buffer[idx].value > -JSDEADZONE) z = 0;
          else if(js_e_buffer[idx].value > 0) z -= JSDEADZONE / zfactor;
          else z += JSDEADZONE / zfactor;
          z = (double)z * DeadzoneFix;
          break;
        default:
          break;
      }
    }
    else
    {// TODO: default: ERROR (non-supported)
    }
  }
  
  /************* at_set_radiogp_input ****************
  * Description : Fill struct radiogp_cmd, 
  * used with at_cmds_loop function.
  * pitch : y-axis (rad) (-25000, +25000)  
  * roll : x-axis (rad) (-25000, +25000) 
  * gaz :  altitude (mm/s) (-25000, +25000)
  * yaw : z-axis (rad/s) (-25000, +25000)
  */
  
  if(refresh_values && (planner == NULL || !planner->enabled))// Axis values to refresh
  {
    ardrone_at_set_radiogp_inputC(y, x, z, yaw); //range checking done in C wrapper
    //PRINT("Pitch:%3d Roll:%3d Gaz:%3d Yaw:%3d\n", y, x, z, yaw);
  }
  
  return C_OK;
}

C_RESULT close_gamepad(void)
{
  close( joy_dev );
  return C_OK;
}

