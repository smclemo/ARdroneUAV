#ifndef _CONTROLS_H
#define _CONTROLS_H

#include "../attitude/attitude.h"
#include "../horizontal_velocities/horizontal_velocities.h"


struct setpoint_struct {
	float pitch; //radians  
	float roll; //radians     
	float yaw; //radians   
	float h; //meters
};


struct control_limits_struct {
	float pitch_roll_max; //radians     
	float h_max; //m
	float h_min; //m
	float throttle_hover; //hover throttle setting
	float throttle_min; //min throttle (while flying)
	float throttle_max; //max throttle (while flying)
};


enum FlyState {
	Landed=1,
	Launching=10,
	Flying=11,
	Landing=12,
	Error=20
};


const char *stateName(enum FlyState state);


struct drone_state_struct {
	struct att_struct att;
	struct horizontal_velocities_struct hor_velocities;
	enum FlyState flyState;
	struct setpoint_struct setpoint;
	struct control_limits_struct control_limits;
};

void switchState(struct drone_state_struct *dronestate, enum FlyState newState);

struct control_strategy_struct {
	/** called once */
	void (*init)();
	/** called every time new attitude data is available, should write desired motor velocities to the float-array */
	void (*calculateMotorSpeeds)(struct drone_state_struct *, float[4]);
	/** should write upto maxLen (second parm) bytes into the char * and return number of bytes written (including \0) */
	unsigned int (*getLogText)(char *,unsigned int);
	void (*Close)();
};

#define LOAD_STRATEGY(targetStruct, name) \
	do { \
		targetStruct.init			= name ## _init; \
		targetStruct.calculateMotorSpeeds	= name ## _calculateMotorSpeeds; \
		targetStruct.getLogText			= name ## _getLogText; \
		targetStruct.Close			= name ## _Close; \
	} while(0)	

#endif
