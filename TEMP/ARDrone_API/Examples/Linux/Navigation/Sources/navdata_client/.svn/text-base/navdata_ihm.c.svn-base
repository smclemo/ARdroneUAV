#include <string.h>
#include <VP_Os/vp_os_malloc.h>
#include <VP_Os/vp_os_print.h>

#include <config.h>

#include "common/common.h"
#include "ihm/ihm.h"
#include "ihm/ihm_vision.h"
#include "ihm/ihm_stages_o_gtk.h"
#include "ihm/view_drone_attitude.h"
#include "navdata_client/navdata_ihm.h"

#ifdef PC_USE_VISION
#     include <Vision/vision_tracker_engine.h>
#endif

#define CTRL_STATES_STRING
#include "control_states.h"

#define SIGN(x) ( x>=0 ? 1: -1)

#ifdef EXTERNAL_CONTROL

#include <mykonos.h>
#include <mykonos_config.h>
#include <ATcodec/ATcodec_api.h>
#include <Acquisition/acq_fsm.h>
#include <Control/control_fsm.h>

extern C_RESULT set_ui_input(bool_t force_landing);
extern bool_t is_ui_ignored(void);
extern int32_t get_ui_misc0( void );

extern mykonos_t mykonos;
extern int32_t iphone_acceleros_enable;
static uint32_t input_check_cpt = COM_INPUT_CHECK_CPT_THRESHOLD; // Make sure we have no input at startup

static uint32_t ctrl_start, ctrl_current;
static int32_t ctrl_watchdog;
static int32_t cpt_50hz;
static int32_t cpt_10hz;
static acq_t acq;
static ctrl_t ctrl = { 0 };

#endif // ! EXTERNAL_CONTROL

drone_angular_rate_references_t wref;//, wtrim;
drone_euler_angles_references_t earef;//, eatrim;
int32_t rc_ref_gaz;
double tab_trims[3];
int32_t ihm_val_idx = 0;

#ifdef PC_USE_VISION
static vp_stages_draw_trackers_config_t draw_trackers_cfg = { 0 };
#endif

const char* ctrl_state_str(uint32_t ctrl_state)
{
  #define MAX_STR_CTRL_STATE 256
  static char str_ctrl_state[MAX_STR_CTRL_STATE];

  ctrl_string_t* ctrl_string;
  uint32_t major = ctrl_state >> 16;
  uint32_t minor = ctrl_state & 0xFFFF;

  if( strlen(ctrl_states[major]) < MAX_STR_CTRL_STATE )
  {
    vp_os_memset(str_ctrl_state, 0, sizeof(str_ctrl_state));

    strcat(str_ctrl_state, ctrl_states[major]);
    ctrl_string = control_states_link[major];

    if( ctrl_string != NULL && (strlen(ctrl_states[major]) + strlen(ctrl_string[minor]) < MAX_STR_CTRL_STATE) )
    {
      strcat( str_ctrl_state, " | " );
      strcat( str_ctrl_state, ctrl_string[minor] );
    }
  }
  else
  {
    vp_os_memset( str_ctrl_state, '#', sizeof(str_ctrl_state) );
  }

  return str_ctrl_state;
}

#ifdef EXTERNAL_CONTROL
  static vision_params_t vision_params;
#endif // ! EXTERNAL_CONTROL

C_RESULT navdata_ihm_init( mobile_config_t* cfg )
{

#ifdef EXTERNAL_CONTROL
  // Watchdog init
  set_mykonos_state(ARDRONE_CTRL_WATCHDOG_MASK, FALSE);
  set_mykonos_state(ARDRONE_ADC_WATCHDOG_MASK,  FALSE);
  set_mykonos_state(ARDRONE_COM_WATCHDOG_MASK,  TRUE);  // No input at startup
  set_mykonos_state(ARDRONE_COMMAND_MASK,       FALSE);
  set_mykonos_state(ARDRONE_TIMER_ELAPSED,      FALSE);

  mykonos.vision = &vision_params;

  //C_RESULT res;
  //adc_t adc;

  //adc.running_fsm     = &acq_fsm;

  acq.n_accs          = mykonos.accs;
  acq.n_gyros         = mykonos.gyros;

  ctrl.euler_angles     = (euler_angles_t*) &mykonos.orientation;
  ctrl.angular_rates    = (angular_rates_t*) mykonos.gyros;

  //fsm_set_param( &adc_fsm, (void*)&adc );
  fsm_set_param( &acq_fsm, (void*)&acq );
  fsm_set_param( &ctrl_fsm, (void*)&ctrl );

  set_mykonos_state(ARDRONE_ACQ_THREAD_ON, TRUE);
  set_mykonos_state(ARDRONE_CONTROL_MASK, FALSE); // EULER ANGLE CONTROL
  set_mykonos_state(ARDRONE_ALTITUDE_MASK, FALSE); // NO ALTITUDE CONTROL

  ctrl_watchdog = 0;
  ctrl_start    = 0;
  ctrl_current  = 0;

  cpt_50hz      = 0;
  cpt_10hz      = 0;
#endif // ! EXTERNAL_CONTROL

  while (!cfg->ihm_curve_alloc_OK) sleep(1);

  return C_OK;
}

C_RESULT navdata_ihm_process( const navdata_unpacked_t* const pnd )
{
  // States' display update for IHM
  vp_os_memset( label_mykonos_state_value, '\0', sizeof(label_mykonos_state_value) );
  sprintf(label_mykonos_state_value, "%08x", pnd->ardrone_state);
#ifndef EXTERNAL_CONTROL
//  strcpy(label_ctrl_state_value, ctrl_state_str( pnd->navdata_demo.ctrl_state ));
//  set_control_state(pnd->navdata_demo.ctrl_state);
#endif // ! EXTERNAL_CONTROL


  ihm_val_idx--;
  if (ihm_val_idx < 0) ihm_val_idx = KIHM_N_PT2PLOT-1;

  // Angular rates received and displayed in deg/s

  ihm_CA[KIHM_CURVE_GYRO].tval[0][ihm_val_idx] = ((double) pnd->navdata_phys_measures.phys_gyros[GYRO_X]);
  ihm_CA[KIHM_CURVE_GYRO].tval[1][ihm_val_idx] = ((double) pnd->navdata_phys_measures.phys_gyros[GYRO_Y]);
  ihm_CA[KIHM_CURVE_GYRO].tval[2][ihm_val_idx] = ((double) pnd->navdata_phys_measures.phys_gyros[GYRO_Z]);

  // Accelerations are received in mg
  // They are displayed in g: conversion gain is 1/1000
  ihm_CA[KIHM_CURVE_ACC].tval[0][ihm_val_idx] = ((double) pnd->navdata_phys_measures.phys_accs[ACC_X]) / KIHM_MIL_F;
  ihm_CA[KIHM_CURVE_ACC].tval[1][ihm_val_idx] = ((double) pnd->navdata_phys_measures.phys_accs[ACC_Y]) / KIHM_MIL_F;
  ihm_CA[KIHM_CURVE_ACC].tval[2][ihm_val_idx] = ((double) pnd->navdata_phys_measures.phys_accs[ACC_Z]) / KIHM_MIL_F;

  // Filtered  temperature is drawn in the Vbat window
  ihm_CA[KIHM_CURVE_VBAT].tval[0][ihm_val_idx] = ((double) (pnd->navdata_phys_measures.accs_temp));   // Filtered temperature

  ihm_CA[KIHM_CURVE_VBAT].tval[1][ihm_val_idx] = ((double) (pnd->navdata_demo.vbat_flying_percentage)) ; // Filtered vbat in %

  ihm_CA[KIHM_CURVE_THETA].tval[2][ihm_val_idx] = ((double) pnd->navdata_euler_angles.theta_a) / KIHM_MIL_F; // theta accelerometer value
  ihm_CA[KIHM_CURVE_PHI  ].tval[2][ihm_val_idx] = ((double) pnd->navdata_euler_angles.phi_a) / KIHM_MIL_F; // phi accelerometer value

  ihm_CA[KIHM_CURVE_THETA].tval[0][ihm_val_idx] = ((double) pnd->navdata_demo.theta) / KIHM_MIL_F; // theta fused value
  ihm_CA[KIHM_CURVE_PHI  ].tval[0][ihm_val_idx] = ((double) pnd->navdata_demo.phi) / KIHM_MIL_F; // phi fused value
  ihm_psi                                       = ((double) pnd->navdata_demo.psi) / KIHM_MIL_F;

  earef.theta = pnd->navdata_references.ref_theta;
  ihm_CA[KIHM_CURVE_THETA].tval[1][ihm_val_idx] = ((double) earef.theta) / KIHM_MIL_F; // theta reference value

  earef.phi = pnd->navdata_references.ref_phi;
  ihm_CA[KIHM_CURVE_PHI  ].tval[1][ihm_val_idx] = ((double) earef.phi) / KIHM_MIL_F; // phi reference value

  earef.psi = pnd->navdata_references.ref_psi;

  wref.p = pnd->navdata_references.ref_roll;
  wref.q = pnd->navdata_references.ref_pitch;
  wref.r = pnd->navdata_references.ref_yaw;

  rc_ref_gaz = pnd->navdata_rc_references.rc_ref_gaz;

  ihm_CA[KIHM_CURVE_ALTITUDE].tval[0][ihm_val_idx] = ((double) pnd->navdata_demo.altitude);

  ihm_CA[KIHM_CURVE_ALTITUDE].tval[1][ihm_val_idx] = ((double) pnd->navdata_altitude.altitude_vision);
  ihm_CA[KIHM_CURVE_ALTITUDE].tval[2][ihm_val_idx] = ((double) pnd->navdata_altitude.altitude_ref);
  ihm_CA[KIHM_CURVE_ALTITUDE].tval[3][ihm_val_idx] = ((double) pnd->navdata_altitude.altitude_raw);

  ihm_CA[KIHM_CURVE_VZ].tval[1][ihm_val_idx] = (double) (pnd->navdata_altitude.altitude_vz);      // Fusion Vz

  tab_trims[0] = (double)pnd->navdata_trims.euler_angles_trim_theta;
  tab_trims[1] = (double)pnd->navdata_trims.euler_angles_trim_phi;
  tab_trims[2] = (double)pnd->navdata_trims.angular_rates_trim_r;

  ihm_CA[KIHM_CURVE_PWM].tval[0][ihm_val_idx] = (double) (pnd->navdata_pwm.motor1); // PWM motor 1
  ihm_CA[KIHM_CURVE_PWM].tval[1][ihm_val_idx] = (double) (pnd->navdata_pwm.motor2); // PWM motor 2
  ihm_CA[KIHM_CURVE_PWM].tval[2][ihm_val_idx] = (double) (pnd->navdata_pwm.motor3); // PWM motor 3
  ihm_CA[KIHM_CURVE_PWM].tval[3][ihm_val_idx] = (double) (pnd->navdata_pwm.motor4); // PWM motor 4
  ihm_CA[KIHM_CURVE_CURRENT].tval[0][ihm_val_idx] = (double) (pnd->navdata_pwm.current_motor1); // current motor 1
  ihm_CA[KIHM_CURVE_CURRENT].tval[1][ihm_val_idx] = (double) (pnd->navdata_pwm.current_motor2); // current motor 2
  ihm_CA[KIHM_CURVE_CURRENT].tval[2][ihm_val_idx] = (double) (pnd->navdata_pwm.current_motor3); // current motor 3
  ihm_CA[KIHM_CURVE_CURRENT].tval[3][ihm_val_idx] = (double) (pnd->navdata_pwm.current_motor4); // current motor 4

  ihm_CA[KIHM_CURVE_VZ].tval[3][ihm_val_idx] = (double) (pnd->navdata_pwm.vz_ref);  // VZ reference

  ihm_CA[KIHM_CURVE_VX].tval[0][ihm_val_idx] = (double) (pnd->navdata_vision_raw.vision_tx_raw);      // VISION trans. en x
  ihm_CA[KIHM_CURVE_VY].tval[0][ihm_val_idx] = (double) (pnd->navdata_vision_raw.vision_ty_raw);      // VISION trans. en y
  ihm_CA[KIHM_CURVE_VZ].tval[0][ihm_val_idx] = (double) (pnd->navdata_vision_raw.vision_tz_raw);      // VISION trans. en z


  if( pnd->vision_defined )
  {
    if( fabsf(pnd->navdata_vision_raw.vision_tx_raw) <= 10.0f && fabsf(pnd->navdata_vision_raw.vision_ty_raw) <= 10.0f )
    {
      ihm_dir = HOVER_VAL;
    }
    else
    {
      ihm_dir = (double) atan2f(pnd->navdata_vision_raw.vision_ty_raw, pnd->navdata_vision_raw.vision_tx_raw);
    }
  }
  else
  {
    ihm_dir = NOT_DEF_VAL;
  }

  //Vision States' display update for IHM
  vp_os_memset( label_vision_state_value, '\0', sizeof(label_vision_state_value) );
  sprintf(label_vision_state_value, "%08x", pnd->navdata_vision.vision_state);

  ihm_CA[KIHM_CURVE_VX].tval[1][ihm_val_idx] = (double) (pnd->navdata_demo.vx);      // Fusion Vx
  ihm_CA[KIHM_CURVE_VY].tval[1][ihm_val_idx] = (double) (pnd->navdata_demo.vy);      // Fusion Vy

#undef PC_USE_VISION
#ifdef PC_USE_VISION
  draw_trackers_cfg.num_points = NUM_MAX_SCREEN_POINTS;
  vp_os_memset(&draw_trackers_cfg.locked[0], 0, NUM_MAX_SCREEN_POINTS * sizeof(uint32_t));
  vp_os_memcpy(&draw_trackers_cfg.locked[0], pnd->navdata_trackers_send.locked, NUM_MAX_SCREEN_POINTS * sizeof(uint32_t));
  vp_os_memset(&draw_trackers_cfg.points[0], 0, NUM_MAX_SCREEN_POINTS * sizeof(screen_point_t));
  vp_os_memcpy(&draw_trackers_cfg.points[0], pnd->navdata_trackers_send.point, NUM_MAX_SCREEN_POINTS * sizeof(screen_point_t));

  int i;
  draw_trackers_cfg.detected = pnd->navdata_vision_detect.nb_detected;
  for(i = 0 ; i < pnd->navdata_vision_detect.nb_detected ; i++)
  {
    draw_trackers_cfg.type[i]           = pnd->navdata_vision_detect.type[i];
    draw_trackers_cfg.patch_center[i].x = pnd->navdata_vision_detect.xc[i];
    draw_trackers_cfg.patch_center[i].y = pnd->navdata_vision_detect.yc[i];
    draw_trackers_cfg.width[i]          = pnd->navdata_vision_detect.width[i];
    draw_trackers_cfg.height[i]         = pnd->navdata_vision_detect.height[i];
  }
  set_draw_trackers_config(&draw_trackers_cfg);
#endif

#undef EXTERNAL_CONTROL
#ifdef EXTERNAL_CONTROL
  C_RESULT res;

  mykonos.vision->defined = pnd->vision_defined;
  mykonos.vision->raw_trans_safe.x = pnd->navdata_vision_raw.vision_tx_raw;
  mykonos.vision->raw_trans_safe.y = pnd->navdata_vision_raw.vision_ty_raw;
  mykonos.vision->raw_trans_safe.z = pnd->navdata_vision_raw.vision_tz_raw;


  res = C_OK;

  //adc.running_fsm     = &acq_fsm;
  acq.data_frame_ptr  = (adc_data_frame_t*)&pnd->navdata_adc_data_frame.data_frame;
  {
    ctrl_start = get_current_time();
    ctrl_start = mykonos_time_to_usec(ctrl_start);

    res = fsm_execute( &acq_fsm );

    // check if user inputs were updated in the last 250ms
    input_check_cpt ++;
    if( !get_mykonos_state(ARDRONE_COM_WATCHDOG_MASK) && COM_INPUT_CHECK_CPT_THRESHOLD < input_check_cpt && is_ui_ignored() == FALSE )
    {
      iphone_acceleros_enable = FALSE;
      ctrl.iPhone_in_use      = FALSE;
      set_ui_input(FALSE);
      set_mykonos_state(ARDRONE_COM_WATCHDOG_MASK, TRUE);
      //syslog_print_emergency(PRINT_WHERE, "Inputs from user are received too slowly (com watchdog)\n");
    }

    // each four iterations we turn on 50hz
    mykonos.flag_50Hz = cpt_50hz == 0 ? TRUE : FALSE;

    cpt_50hz ++;
    cpt_50hz &= 3; // acquisition are running at 200Hz

    // each twenty iterations we turn on 10hz
    mykonos.flag_10Hz = cpt_10hz == 0 ? TRUE : FALSE;

    cpt_10hz ++;
    if( 20 == cpt_10hz )
      cpt_10hz = 0;
    // cpt_10hz = cpt_10hz %20; // acquisition are running at 200Hz

    if( get_mykonos_state( ARDRONE_ACQ_THREAD_ON ) && SUCCEED(res) )
    {
      // TODO Check if we can pass orientation as a ptr in imu_state
      mykonos.orientation = acq.imu_state.angles.fused;

      // After control execution we test fused euler angles
      // to check if they are out of range
      // In this case an emergency landing signal is sent
      check_euler_angles();

      //#ifdef EXTERNAL_CONTROL
      //      NAVDATA_SET_ADC_DATA_FRAME( &adc.data_frame );
      //#endif // ! EXTERNAL_CONTROL

      if ( acq_fsm.state == ACQ_RUN )
      {
	//#ifndef EXTERNAL_CONTROL
        fsm_execute( &ctrl_fsm );
	//#endif // ! EXTERNAL_CONTROL

        ctrl_current = get_current_time();
	ctrl_current = mykonos_time_to_usec(ctrl_current);
        ctrl_watchdog = (int32_t)ctrl_current - (int32_t)ctrl_start;
      }

      switch( get_ui_misc0() )
      {
        case NO_CONTROL:
          set_mykonos_state(ARDRONE_ALTITUDE_MASK, FALSE); // NO ALTITUDE CONTROL
          set_mykonos_state(ARDRONE_VISION_MASK, FALSE);   // NO VIDEO CONTROL
          break;

        case ALTITUDE_CONTROL:
          set_mykonos_state(ARDRONE_ALTITUDE_MASK, TRUE); // ALTITUDE CONTROL
          set_mykonos_state(ARDRONE_VISION_MASK, FALSE);   // NO VIDEO CONTROL
          break;

        case ALTITUDE_VISION_CONTROL:
        case ALTITUDE_VISION_CONTROL_TAKEOFF_TRIM:
        default:
          set_mykonos_state(ARDRONE_ALTITUDE_MASK, TRUE); // ALTITUDE CONTROL
          set_mykonos_state(ARDRONE_VISION_MASK, TRUE);    // VIDEO CONTROL
          break;
      }
    }
    else
    {
      motors_stop_all();
    }
  }
#endif // ! EXTERNAL_CONTROL

  return C_OK;
}

C_RESULT navdata_ihm_release( void )
{
#ifdef EXTERNAL_CONTROL
  motors_stop_all();

  //set_landed_timer(LANDED_TIMER_PERIOD, landed_toy_timer_event_cb);

  set_mykonos_state(ARDRONE_NAVDATA_THREAD_ON, FALSE);
  set_mykonos_state(ARDRONE_ACQ_THREAD_ON, FALSE);

  //syslog_print_info(PRINT_WHERE, "Thread Acquisition End\n");
#endif // ! EXTERNAL_CONTROL

  return C_OK;
}

