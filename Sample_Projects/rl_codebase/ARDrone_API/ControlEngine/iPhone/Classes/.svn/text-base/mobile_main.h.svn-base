/*
 *  mobile_main.h
 *  Test
 *
 *  Created by Karl Leplat on 19/02/10.
 *  Copyright 2010 Parrot SA. All rights reserved.
 *
 */
#ifndef _MOBILE_MAIN_H_
#define _MOBILE_MAIN_H_

#include "ConstantsAndMacros.h"

typedef enum
{
	ARDRONE_ENGINE_INIT_OK,
	ARDRONE_ENGINE_MAX
} ARDRONE_ENGINE_MESSAGE;

typedef void (*ardroneEngineCallback)(ARDRONE_ENGINE_MESSAGE msg);

#ifdef _cplusplus
extern "C" {
#endif
	void ardroneEnginePause( void );
	void ardroneEngineResume( void );
	void ardroneEngineStart( ardroneEngineCallback callback );
	void ardroneEngineStop( void );
#ifdef _cplusplus
}
#endif		

#endif