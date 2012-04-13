/*
 * AR Drone demo
 *
 * originally based on Android NDK "San Angeles" demo app
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include <sys/time.h>
#include <time.h>

#include "app.h"

static int gAppAlive = 0;

void appInit()
{
	//Run Stream process
	at_run();
	navdata_run();
	stream_run();

	gAppAlive = 1;
}

void appDeinit()
{
	INFO("shutting down application...\n");

	gAppAlive = 0;

	// all threads should implement a loop polling gAppAlive
	stream_stop();

	INFO("application was cleanly exited\n");
}
