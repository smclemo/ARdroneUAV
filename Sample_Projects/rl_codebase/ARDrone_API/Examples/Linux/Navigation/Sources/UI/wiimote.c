/**
 *  \brief    Wiimote handling implementation
 *  \author   Aurelien Morelle <aurelien.morelle@parrot.com>
 *  \version  1.0
 *  \date     04/06/2007
 */

#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <stddef.h>
#include <stdio.h>
#include <unistd.h>

#include "UI/wiimote.h"


input_device_t wiimote_device = {
  "Wiimote",
  open_wiimote,
  update_wiimote,
  close_wiimote
};


#ifndef WIIMOTE_SUPPORT

C_RESULT open_wiimote(void)
{
  return C_FAIL;
}

C_RESULT update_wiimote(void)
{
  return C_FAIL;
}

C_RESULT close_wiimote(void)
{
  return C_OK;
}

#else // ! WIIMOTE_SUPPORT

#include <bluetooth/bluetooth.h>
#include <VP_Os/vp_os_print.h>
#include "libcwiid/cwiid.h"

static bdaddr_t bdaddr = {0};
static cwiid_wiimote_t *wiimote = NULL;

static struct acc_cal wm_cal;

C_RESULT open_wiimote(void)
{
  C_RESULT res = C_FAIL;

  if(cwiid_find_wiimote(&bdaddr, WIIMOTE_FIND_TIMEOUT) == 0)
  {
    if( ! (wiimote = cwiid_open(&bdaddr, 0)) )
      PRINT("Unable to connect to wiimote\n");
    else
      res = C_OK;
  }

  if(SUCCEED(res))
    if(cwiid_set_rpt_mode(wiimote, CWIID_RPT_BTN|CWIID_RPT_ACC))
      res = C_FAIL;

  if(SUCCEED(res))
    if (cwiid_get_acc_cal(wiimote, CWIID_EXT_NONE, &wm_cal))
      res = C_FAIL;

  return res;
}

void print_state(struct cwiid_state *state)
{
  printf("Battery: %d%%\n", (int)(100.0 * state->battery / CWIID_BATTERY_MAX));
  printf("Buttons: %X\n", state->buttons);
  printf("Acc: x=%d y=%d z=%d\n", state->acc[CWIID_X], state->acc[CWIID_Y], state->acc[CWIID_Z]);
}

extern void api_set_iphone_acceleros(bool_t enable, float32_t fax, float32_t fay, float32_t faz);

#define NEW_AMOUNT 0.1
#define OLD_AMOUNT (1.0-NEW_AMOUNT)
static float32_t a_x = 0, a_y = 0, a_z = 0;

C_RESULT update_wiimote(void)
{
  C_RESULT res = C_OK;
  struct cwiid_state state;
  int8_t x, y;

  if (cwiid_get_state(wiimote, &state))
    {
      fprintf(stderr, "Error getting state\n");
      res = C_FAIL;
    }

/*   mykonos_tool_set_ui_pad_ag(state.buttons&CWIID_BTN_1 ? 1 : 0); // shoot */
/*   mykonos_tool_set_ui_pad_ab(state.buttons&CWIID_BTN_LEFT ? 1 : 0); // altitude -- */
/*   mykonos_tool_set_ui_pad_ad(state.buttons&CWIID_BTN_MINUS ? 1 : 0); // mayday */
/*   mykonos_tool_set_ui_pad_ah(state.buttons&CWIID_BTN_RIGHT ? 1 : 0); // altitude ++ */

/*   mykonos_tool_set_ui_pad_l1(state.buttons&CWIID_BTN_UP ? 1 : 0); // yaw -- */
/*   mykonos_tool_set_ui_pad_r1(state.buttons&CWIID_BTN_DOWN ? 1 : 0); // yaw ++ */
/*   mykonos_tool_set_ui_pad_l2(state.buttons&CWIID_BTN_B ? 1 : 0); // misc */
/*   mykonos_tool_set_ui_pad_r2(0 /\*state.buttons&CWIID_BTN_A ? 1 : 0*\/); // trim */

/*   mykonos_tool_set_ui_pad_select(state.buttons&CWIID_BTN_A ? 1 : 0); // motors off */
/*   mykonos_tool_set_ui_pad_start(state.buttons&CWIID_BTN_PLUS ? 1 : 0); // take off / land */

  mykonos_tool_set_ui_pad_l2(state.buttons&CWIID_BTN_B ? 1 : 0); // misc
  mykonos_tool_set_ui_pad_select(state.buttons&CWIID_BTN_PLUS ? 1 : 0); // motors off
  mykonos_tool_set_ui_pad_start(state.buttons&CWIID_BTN_MINUS ? 1 : 0); // take off / land

  a_x = (float32_t) ((((double)state.acc[CWIID_X] - wm_cal.zero[CWIID_X]) /
		      (wm_cal.one[CWIID_X] - wm_cal.zero[CWIID_X])));
  a_y = (float32_t) ((((double)state.acc[CWIID_Y] - wm_cal.zero[CWIID_Y]) /
		      (wm_cal.one[CWIID_Y] - wm_cal.zero[CWIID_Y])));
  a_z = - (float32_t) ((((double)state.acc[CWIID_Z] - wm_cal.zero[CWIID_Z]) /
			(wm_cal.one[CWIID_Z] - wm_cal.zero[CWIID_Z])));

  api_set_iphone_acceleros(
			   (state.buttons&CWIID_BTN_2 ? 1 : 0)|(state.buttons&CWIID_BTN_A ? 2 : 0),
			   a_x, a_y, a_z);

  return C_OK;
}

C_RESULT close_wiimote(void)
{
  cwiid_close(wiimote);

  return C_OK;
}

#endif // ! WIIMOTE_SUPPORT

