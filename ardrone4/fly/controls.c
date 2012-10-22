#include "controls.h"
#include <stdio.h>

struct setpoint_struct setpoint;
struct control_limits_struct control_limits;

enum FlyState flyState;

const char *stateName(enum FlyState state)
{
	switch(state) {
	  case Landed: return "Landed"; break;
	  case Launching: return "Launching"; break;
	  case Flying: return "Flying"; break;
	  case Landing: return "Landing"; break;
	  case Error: return "Error"; break;
	  default: return "Unknown"; break;
	}
}




void switchState(struct drone_state_struct * dronestate, enum FlyState newState)
{
  printf("Switching state to %d: %s\n",newState, stateName(newState));
  dronestate->flyState=newState;
}
