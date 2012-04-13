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
#include <android/log.h>

#include "app.h"

pthread_t nav_thread = 0;
pthread_t at_thread = 0;
pthread_t stream_thread = 0;

char drone_ip[24];
int gAppAlive   = 0;

// assume drone has address x.x.x.1
#define WIFI_MYKONOS_ADDR_LAST_BYTE (1)

void get_drone_ip(void)
{
    int ret;
    char prop_value[PROP_VALUE_MAX];
    char prop_name[PROP_NAME_MAX];
    int inet_part[4];

    // quick and dirty way to guess Drone IP address
    ret = GETPROP("wifi.interface", prop_value);
    if (ret > 0) {
        INFO("wifi interface = %s\n", prop_value);
        snprintf(prop_name, PROP_NAME_MAX, "dhcp.%s.ipaddress", prop_value);
        ret = GETPROP(prop_name, prop_value);
    }
    if (ret > 0) {
        INFO("IP address = %s\n", prop_value);
        ret = 0;
        if (sscanf(prop_value, "%d.%d.%d.%d",
                   &inet_part[0],
                   &inet_part[1],
                   &inet_part[2],
                   &inet_part[3]) == 4) {
            ret = 1;
            sprintf(drone_ip, "%u.%u.%u.%u",
                    inet_part[0]& 0xff, inet_part[1]& 0xff, inet_part[2]&0xff,
                    WIFI_MYKONOS_ADDR_LAST_BYTE);
        }
    }
    if (ret == 0) {
        // fallback to default address
        sprintf(drone_ip, WIFI_MYKONOS_IP);
    }

    INFO("assuming drone IP address is %s\n", drone_ip);
}

void appInit(void)
{
    int status;

    get_drone_ip();

    gAppAlive = 1;

    // navigation
    if (!nav_thread) {
        status = pthread_create(&nav_thread, NULL, navdata_loop, NULL);
        if (status) {
            INFO("pthread_create: %s\n", strerror(errno));
        }
    }

    // video stream
    if (!stream_thread) {
        status = pthread_create(&stream_thread, NULL, stream_loop, NULL);
        if (status) {
            INFO("pthread_create: %s\n", strerror(errno));
        }
    }

    // AT cmds loop
    if (!at_thread) {
        status = pthread_create(&at_thread, NULL, at_cmds_loop, NULL);
        if (status) {
            INFO("pthread_create: %s\n", strerror(errno));
        }
    }

    // video rendering
    video_init();
}

void appDeinit()
{
    gAppAlive = 0;

    INFO("shutting down application...\n");

    video_deinit();

    // all threads should implement a loop polling gAppAlive
    pthread_join(nav_thread, NULL);
    pthread_join(at_thread, NULL);
    pthread_join(stream_thread, NULL);

    nav_thread = 0;
    at_thread = 0;
    stream_thread = 0;

    INFO("application was cleanly terminated\n");
}

void appRender(long tick, int width, int height)
{
    if (!gAppAlive) {
        return;
    }
    video_render(tick, width, height);
}
