#ifndef _GAMEPAD_H_
#define _GAMEPAD_H_

#include "UI/ui.h"
#include "keyboard.h"

#define GAMEPAD_LOGICTECH_ID 0x046dc21a

#define GAMEPAD_OVERRIDE 1 //if non-zero, then gamepad ID given below will be used instead of searching dev devices
#define GAMEPAD_OS_ID "js0"
#define JSDEADZONE 6000 //deadzone to use with the joystick (0 for joystick, 6000 for xbox controller, modify as nessasary).
#define XBOXCONTROLLER 1 //if non-zero, then set up axis for an xbox 360 controller, otherwise use joystick axis layout

#if XBOXCONTROLLER
typedef enum {
  PAD_YAW,
  PAD_Z,
  PAD_NULL,
  PAD_X,
  PAD_Y
} PAD_AXIS;
#else
typedef enum {
  PAD_X,
  PAD_Y,
  PAD_YAW,
  PAD_Z
} PAD_AXIS;
#endif

typedef enum {
  PAD_AG = 0,
  PAD_AB,
  PAD_AD,
  PAD_AH,
  PAD_L1,
  PAD_R1,
  PAD_L2,
  PAD_R2,
  PAD_SELECT,
  PAD_START,
  PAD_NUM_BUTTONS
} PAD_BUTTONS;

extern input_device_t gamepad;

C_RESULT open_gamepad(void);
C_RESULT update_gamepad(void);
C_RESULT close_gamepad(void);

#define RADIO_GP_ID 0x061c0010

typedef enum _TYPE
{
  TYPE_MIN      =  0,
  TYPE_BUTTON   =  1,
  TYPE_ANALOGIC =  2,
  TYPE_MAX      =  3
} TYPE;

typedef enum _ANALOGIC_RADIO_COMMAND_GP
{
  GP_ROLL  =  0,
  GP_GAZ   =  1,
  GP_PITCH =  2,
  GP_PID   =  3,
  GP_YAW   =  4,
  GP_ANALOG_MAX   =  5,
} ANALOGIC_RADIO_COMMAND_GP;

#define  NUM_A_GP_MIN GP_ROLL
#define  NUM_A_GP_MAX GP_MAX

#define OFFSET_PITCH_GP 127
#define OFFSET_ROLL_GP  127
#define OFFSET_YAW_GP   127
#define OFFSET_GAZ_GP   198

#define NUM_PITCH_GP     -3
#define NUM_ROLL_GP       3
#define NUM_YAW_GP        3
#define NUM_GAZ_GP       -7

#define DEC_PITCH_GP      1
#define DEC_ROLL_GP       1
#define DEC_YAW_GP        1
#define DEC_GAZ_GP        2


typedef enum _BUTTON_RADIO_COMMAND_GP
{
  GP_BOARD_LEFT      =   0, // switch above left joystick
  GP_SIDE_RIGHT      =   1, // right corner switch
  GP_IMPULSE         =   2, // Red button
  GP_SIDE_LEFT_DOWN  =   3, // left corner switch, down position
  GP_SIDE_LEFT_UP    =   4, // left corner switch, up position
  GP_BUTTON_MAX             =   5
} BUTTON_RADIO_COMMAND_GP;

#define  NUM_B_GP_MIN GP_BOARD_LEFT
#define  NUM_B_GP_MAX GP_MAX
extern input_device_t radioGP;

C_RESULT open_radioGP(void);
C_RESULT update_radioGP(void);
C_RESULT close_radioGP(void);

#endif // _GAMEPAD_H_
