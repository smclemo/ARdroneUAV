/*
    navdata.h - navdata driver
*/ 
#ifndef _NAVDATA_H
#define _NAVDATA_H


#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <netinet/in.h>
#include <netdb.h>
#include <errno.h>
#include <arpa/inet.h>
#include <assert.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#include <sys/time.h>

#include <fcntl.h>
#include <sys/ioctl.h>
#include <net/if.h>

#include "app.h"
#include "util.h"

/* Navdata constant */
#define NAVDATA_SEQUENCE_DEFAULT  1
#define NAVDATA_PORT              5554
#define NAVDATA_HEADER            0x55667788
#define NAVDATA_BUFFER_SIZE       2048

#define REMOTE_SERVER_ADDRESS   "192.168.1.1"


/**
 * @def ardrone_navdata_unpack
 * @brief Extract an'option' from the navdata network packet sent by the drone.
 * Used by the client 'navdata' thread inside ARDroneTool.
*/
#define ardrone_navdata_unpack( navdata_ptr, option ) (navdata_option_t*) navdata_unpack_option( (uint8_t*) navdata_ptr, \
                                                                                         navdata_ptr->size,              \
                                                                                         (uint8_t*) &option,             \
                                                                                         sizeof (option) )
/**
 * @fn navdata_unpack_option
 * @brief Extract an 'option' from the navdata network packet sent by the drone.
 * Used by the client 'navdata' thread inside ARDroneTool.
*/
static INLINE uint8_t* navdata_unpack_option( uint8_t* navdata_ptr, uint32_t ptrsize, uint8_t* data, uint32_t datasize )
{
  uint32_t minSize = (ptrsize < datasize) ? ptrsize : datasize;
  memcpy(data, navdata_ptr, minSize);
  return (navdata_ptr + ptrsize);
}
/*

typedef enum _navdata_tag_t 
{
    NAVDATA_DEMO_TAG = 0,
    NAVDATA_VISION_DETECT_TAG = 16,
    NAVDATA_IPHONE_ANGLES_TAG = 18,
    NAVDATA_CKS_TAG = 0xFFFF
} navdata_tag_t;

typedef struct _matrix33_t
{ 
    float32_t m11;
    float32_t m12;
    float32_t m13;
    float32_t m21;
    float32_t m22;
    float32_t m23;
    float32_t m31;
    float32_t m32;
    float32_t m33;
} matrix33_t;

typedef struct _vector31_t 
{
    union {
        float32_t v[3];
        struct    
        {
            float32_t x;
            float32_t y;
            float32_t z;
        };
    };
} vector31_t; 

typedef struct _navdata_option_t 
{
    uint16_t  tag;
    uint16_t  size;
    uint8_t   data[];
} navdata_option_t;

typedef struct _navdata_t 
{
    uint32_t    header;
    uint32_t    mykonos_state;
    uint32_t    sequence;
    int      vision_defined;

    navdata_option_t  options[1];
} __attribute__ ((packed)) navdata_t;

typedef struct _navdata_cks_t 
{
    uint16_t  tag;
    uint16_t  size;

    // Checksum for all navdatas (including options)
    uint32_t  cks;
} __attribute__ ((packed)) navdata_cks_t;

typedef struct _navdata_demo_t 
{
    uint16_t    tag;
    uint16_t    size;

    uint32_t    ctrl_state;             /*!< instance of #def_mykonos_state_mask_t 
    uint32_t    vbat_flying_percentage; /*!< battery voltage filtered (mV) 

    float32_t   theta;                  /*!< UAV's attitude 
    float32_t   phi;                    /*!< UAV's attitude 
    float32_t   psi;                    /*!< UAV's attitude 

    int32_t     altitude;               /*!< UAV's altitude 

    float32_t   vx;                     /*!< UAV's estimated linear velocity 
    float32_t   vy;                     /*!< UAV's estimated linear velocity 
    float32_t   vz;                     /*!< UAV's estimated linear velocity 

    uint32_t    num_frames;			  /*!< streamed frame index 

    // Camera parameters compute by detection
    matrix33_t  detection_camera_rot;
    matrix33_t  detection_camera_homo;
    vector31_t  detection_camera_trans;

    // Camera parameters compute by drone
    matrix33_t  drone_camera_rot;
    vector31_t  drone_camera_trans;
} __attribute__ ((packed)) navdata_demo_t;

typedef struct _navdata_iphone_angles_t 
{
    uint16_t   tag;
    uint16_t   size;

    int32_t    enable;
    float32_t  ax;
    float32_t  ay;
    float32_t  az;
    uint32_t   elapsed;
} __attribute__ ((packed)) navdata_iphone_angles_t;

typedef struct _navdata_time_t 
{
    uint16_t  tag;
    uint16_t  size;
  
    uint32_t  time;
} __attribute__ ((packed)) navdata_time_t;

typedef struct _navdata_vision_detect_t 
{
    uint16_t   tag;
    uint16_t   size;
  
    uint32_t   nb_detected;  
    uint32_t   type[4];
    uint32_t   xc[4];        
    uint32_t   yc[4];
    uint32_t   width[4];     
    uint32_t   height[4];    
    uint32_t   dist[4];      
} __attribute__ ((packed)) navdata_vision_detect_t;

typedef struct _navdata_unpacked_t 
{
    uint32_t  mykonos_state;
    int    vision_defined;

    navdata_demo_t           navdata_demo;
    navdata_iphone_angles_t  navdata_iphone_angles;
    navdata_vision_detect_t  navdata_vision_detect;
} navdata_unpacked_t;
*/
#endif
