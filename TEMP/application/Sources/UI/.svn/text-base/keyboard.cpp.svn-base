/**
 *  \brief    keyboard handling implementation
 *  \author   Cooper Bills <csb88@cornell.edu>
 *  \version  1.0
 *  \date     06/09/2010
 */

#include <stdlib.h>
#include <sys/select.h>

#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <stddef.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>

#include "keyboard.h"
extern "C" {
#include <VP_Os/vp_os_print.h>
#include <ardrone_api.h>
#include "ardrone_testing_tool.h"
}

#include "planner.hpp"
#include "terminalinput.hpp"

extern int CAPTUREIMAGESTREAM;
extern FILE *commandLog;

input_device_t keyboard = {
  "Keyboard",
  open_keyboard,
  update_keyboard,
  close_keyboard
};

//Function wrappers to allow C calls in C++:
extern "C" {
  extern void ardrone_at_zapC(ZAP_VIDEO_CHANNEL channel);
  extern void start_stop();
  extern int ardrone_at_set_radiogp_inputC(int32_t y, int32_t x, int32_t z, int32_t yaw);

}

////////////////////////////////
//  Keyboard input functions  //
////////////////////////////////

Planner *planner;
TerminalInput *kbInput;

C_RESULT open_keyboard(void)
{

  if(planner == NULL) planner = new Planner();

  kbInput = new TerminalInput();
  /* Keyboard input is currently broken, so hide control menu.
  printf("*************************************\n");
  printf("* Keyboard Controls:\n");
  printf("*   awsd - gaz/yaw\n");
  printf("*   jikl - roll/pitch\n");
  printf("*  \n");
  printf("*   backspace - reset keyboard controls\n");
  printf("*   z - (zap) toggle viewport/camera\n");
  printf("*   b - capture image to disk\n");
  printf("*   v - capture image stream to disk\n");
  printf("*   t - takoff/land ARDrone\n");
  printf("*   Space - toggle Algorithm control\n");
  printf("*   c - flush log buffers (save to disk)\n");
  printf("*************************************\n");
  */
  return C_OK;
}

#define KEYBOARDRATE 1000
static int32_t xb = 0, yb = 0, yawb = 0, zb = 0;
C_RESULT update_keyboard(void)
{
  static bool_t refresh_values = FALSE;
  //If algorithms are enabled, update them before checking joystick:
  //*****ALGORITHM CONTROL*****
  if(planner->enabled)
  {
    yb = planner->dpitch_final;
    xb = planner->droll_final;
    yawb = planner->dyaw_final;
    zb = planner->dgaz_final;
    ardrone_at_set_radiogp_inputC(yb, xb, zb, yawb); //range checking done in C wrapper
  }

  char key = 0;
  if(kbInput->ready())
  {
    PRINT("IN-KBHIT!\n");
    key = kbInput->getCh();
    key = tolower(key);
  }
  
  refresh_values = FALSE;
  //******KEYBOARD COMMANDS******
  switch( key )
  {
    case 0x62 : //b to dump image
      char filename[256];
      timeval t;
      gettimeofday(&t, NULL);
      sprintf(filename, "%d.%06d.bmp", (int)t.tv_sec, (int)t.tv_usec);
      if(frontImgStream != NULL && cvSaveImage(filename, cvCloneImage(frontImgStream)))
        printf("Image dumped to %s\n", filename);
      else
        printf("Error dumping image.\n");
      break;
    case 'v' : //v to toggle image stream capture
      CAPTUREIMAGESTREAM = CAPTUREIMAGESTREAM ? false : true;
      if(CAPTUREIMAGESTREAM)
        printf("Starting Image Stream Capture...\n");
      else
        printf("Stopping Image Stream Capture...\n");
      break;
    case 'z' : //z for viewport toggle
      ardrone_at_zapC(ZAP_CHANNEL_NEXT);
      break;
    case 't' : //enter for start/stop
      PRINT("********************************************Starting/Landing...\n");
      start_stop();
      break;
    case 'c' :
      if(commandLog != NULL)
      {
        fflush(commandLog);
        printf("commandLog flushed.\n");
      }
      break;
    case 'p':
      ardrone_at_set_pwm(24999, 0, 0, 0);
      break;
    case 0x20 : //spacebar enables/disables algorithms
      ardrone_at_set_flight_without_shell(FALSE);
      ardrone_at_set_outdoor(FALSE);
      planner->enabled = planner->enabled ? FALSE : TRUE;
      if(planner->enabled) PRINT("Algorithms Enabled.\n"); else PRINT("Algorithms Disabled.\n");
      ardrone_at_set_radiogp_inputC(0, 0, 0, 0); //reset
      break;
    default:
      break;
  }
  
  //******KEYBOARD CONTROL******
  if(!planner->enabled)
  {
    switch( key )
    {
      case 'a' :
        yawb -= KEYBOARDRATE;
        refresh_values = TRUE;
        break;
      case 'd' :
        yawb += KEYBOARDRATE;
        refresh_values = TRUE;
        break;
      case 'w' :
        zb -= KEYBOARDRATE;
        refresh_values = TRUE;
        break;
      case 's' :
        zb += KEYBOARDRATE;
        refresh_values = TRUE;
        break;
      case 'j' :
        yb -= KEYBOARDRATE;
        refresh_values = TRUE;
        break;
      case 'l' :
        yb += KEYBOARDRATE;
        refresh_values = TRUE;
        break;
      case 'i' :
        xb -= KEYBOARDRATE;
        refresh_values = TRUE;
        break;
      case 'k' :
        xb += KEYBOARDRATE;
        refresh_values = TRUE;
        break;
      case 0x7F : //backspace resets trim
        xb = 0;
        yb = 0;
        yawb = 0;
        zb = 0;
        refresh_values = TRUE;
      default:
        break;
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
  
  if(refresh_values && !planner->enabled)// Axis values to refresh
  {
    ardrone_at_set_radiogp_inputC(yb, xb, zb, yawb); //range checking done in C wrapper
  }
  
  return C_OK;
}

C_RESULT close_keyboard(void)
{
  reset_terminal_mode();

  return C_OK;
}

