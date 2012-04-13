/*
 * AR Drone demo
 *
 * code originally nased on:"San Angeles" Android demo app
 */

#include <jni.h>
#include <sys/time.h>
#include <time.h>
#include <android/log.h>
#include <stdint.h>
#include "app.h"

static int  sWindowWidth  = 320;
static int  sWindowHeight = 480;
static int  sDemoStopped  = 0;
static long sTimeOffset   = 0;
static int  sTimeOffsetInit = 0;
static long sTimeStopped  = 0;

static long
_getTime(void)
{
    struct timeval  now;

    gettimeofday(&now, NULL);
    return (long)(now.tv_sec*1000 + now.tv_usec/1000);
}

/* Call to initialize the graphics state */
void
Java_com_parrot_ARDrone_DemoRenderer_nativeInit( JNIEnv*  env )
{
    //importGLInit();
    appInit();
    sDemoStopped = 0;
    sTimeOffsetInit = 0;
}

void
Java_com_parrot_ARDrone_DemoRenderer_nativeResize( JNIEnv*  env, jobject  thiz, jint w, jint h )
{
    sWindowWidth  = w;
    sWindowHeight = h;
    __android_log_print(ANDROID_LOG_INFO, "ARDrone", "resize w=%d h=%d", w, h);
}

/* Call to finalize the graphics state */
void
Java_com_parrot_ARDrone_DemoRenderer_nativeDone( JNIEnv*  env )
{
    //appDeinit();
    //importGLDeinit();
}

/* Call to render the next GL frame */
void
Java_com_parrot_ARDrone_DemoRenderer_nativeRender( JNIEnv*  env )
{
    long   curTime;

    /* NOTE: if sDemoStopped is TRUE, then we re-render the same frame
     *       on each iteration.
     */
    if (sDemoStopped) {
        curTime = sTimeStopped + sTimeOffset;
    } else {
        curTime = _getTime() + sTimeOffset;
        if (sTimeOffsetInit == 0) {
            sTimeOffsetInit = 1;
            sTimeOffset     = -curTime;
            curTime         = 0;
        }
    }

    //__android_log_print(ANDROID_LOG_INFO, "ARDrone", "curTime=%ld", curTime);

    appRender(curTime, sWindowWidth, sWindowHeight);
}

/* This is called to indicate to the render loop that it should
 * stop as soon as possible.
 */
void
Java_com_parrot_ARDrone_DemoGLSurfaceView_nativePause( JNIEnv*  env )
{
    sDemoStopped = !sDemoStopped;
    if (sDemoStopped) {
        /* we paused the animation, so store the current
         * time in sTimeStopped for future nativeRender calls */
        sTimeStopped = _getTime();
    } else {
        /* we resumed the animation, so adjust the time offset
         * to take care of the pause interval. */
        sTimeOffset -= _getTime() - sTimeStopped;
    }
}

#define TRACKBALL_THRESHOLD 32

void
Java_com_parrot_ARDrone_DemoGLSurfaceView_nativeTrackballEvent(JNIEnv *env,
                                                               jobject thiz,
                                                               jlong eventTime,
                                                               jint action,
                                                               jfloat x,
                                                               jfloat y) {
	static int nx = 0;

	//INFO("track %ld %d (%d,%d)\n", (long)eventTime, (int)action, (int)x, (int)y);

	//FIXME: thread synchronization required here !!

	if (action == 0 /* press */) {
		trackball_info.state = 1;
	}
	if (action == 1 /* release */) {
		trackball_info.state = 0;
	}
	/* horizontal gestures = YAW control
	 * x = 0: left
	 * x = 1: idle position
	 * x = 2: right
	 */
	nx += (int)(100.0*x);

	// FIXME: use trackball to control yaw in a better way
	if (nx <= -TRACKBALL_THRESHOLD) {
		trackball_info.x = (trackball_info.x == 0)? 1 : 2;
		nx = 0;
	}
	else if (nx >= TRACKBALL_THRESHOLD) {
		trackball_info.x = (trackball_info.x == 2)? 1 : 0;
		nx = 0;
	}
}

void
Java_com_parrot_ARDrone_DemoGLSurfaceView_nativeMotionEvent(JNIEnv *env,
                                                            jobject thiz,
                                                            jlong eventTime,
                                                            jint action,
                                                            jfloat x,
                                                            jfloat y) {
	//FIXME: thread synchronization required here !!

	if (action == 1 /* release */) {
		motion_info.state = 0;
	}
	else {
		motion_info.state = 1;
	}
    motion_info.x = (int)(x);
    motion_info.y = (int)(y);

	/*
	if (motion_info.state == 1) {
		INFO("touch %ld @(%d,%d)\n", (long)eventTime,(int)x, (int)y);
	}
	*/
}
/*
void
Java_com_parrot_ARDrone_DemoGLSurfaceView_nativeKeyEvent(JNIEnv *env,
														 jobject thiz,
														 jint action) {
	//FIXME: thread synchronization required here !!
	trackball_info.state = (int)action;
	INFO("KEY %d\n", (int)action);
}
*/

void
Java_com_parrot_ARDrone_DemoActivity_nativeSensorEvent(JNIEnv *env,
													   jobject thiz,
													   jfloat x,
													   jfloat y,
													   jfloat z) {
	//FIXME: thread synchronization required here !!
	orientation.values[0] = (int)x;
	orientation.values[1] = (int)y;
	orientation.values[2] = (int)z;
}

void
Java_com_parrot_ARDrone_DemoActivity_nativeStop( JNIEnv*  env )
{
    appDeinit();
}
