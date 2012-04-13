#ifndef _KEYBOARD_H_
#define _KEYBOARD_H_

#include "UI/ui.h"

#define KEYBOARD_ID 0x0F0F0F0F

extern input_device_t keyboard;

C_RESULT open_keyboard(void);
C_RESULT update_keyboard(void);
C_RESULT close_keyboard(void);

#endif // _KEYBOARD_H_
