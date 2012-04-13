/*
 *  ARDroneTypes.h
 *  ARDroneEngine
 *
 *  Created by Frédéric D'HAEYER on 21/05/10.
 *  Copyright 2010 Parrot SA. All rights reserved.
 *
 */

/**
 * Define Number of enemies
 */
#define ARDRONE_MAX_ENEMIES		5

/**
 * Define the command identifiers from drone to Game Engine
 */
typedef enum {
	ARDRONE_COMMAND_RUN, 
	ARDRONE_COMMAND_PAUSE, 
	ARDRONE_COMMAND_FIRE,
} ARDRONE_COMMAND_OUT;

/**
 * Define the command identifiers from Game Engine to drone
 */
typedef enum {
	ARDRONE_COMMAND_ISCLIENT,			// Command to set if the multiplayer is client
	ARDRONE_COMMAND_DRONE_ANIM,			// Command to set a drone animation
	ARDRONE_COMMAND_DRONE_LED_ANIM,     // Command to set a drone led animation
	ARDRONE_COMMAND_VIDEO_CHANNEL,		// Command to set the channel of video 
	ARDRONE_COMMAND_CAMERA_DETECTION,	// Command to set camera type for detection and tag size. 
} ARDRONE_COMMAND_IN;

/**
 * Define parameter for the command to animate drone
 */
typedef enum {
	ARDRONE_ANIMATION_PHI_M30_DEG= 0,
	ARDRONE_ANIMATION_PHI_30_DEG,
	ARDRONE_ANIMATION_THETA_M30_DEG,
	ARDRONE_ANIMATION_THETA_30_DEG,
	ARDRONE_ANIMATION_THETA_20DEG_YAW_200DEG,
	ARDRONE_ANIMATION_THETA_20DEG_YAW_M200DEG,
	ARDRONE_ANIMATION_TURNAROUND,
	ARDRONE_ANIMATION_TURNAROUND_GODOWN,
	ARDRONE_ANIMATION_YAW_SHAKE,
	ARDRONE_ANIMATION_MAX
} ARDRONE_ANIMATION;

/**
 * Define parameter for the command to switch the channel of video
 */
typedef enum {
	ARDRONE_VIDEO_CHANNEL_FIRST = 0,
	ARDRONE_VIDEO_CHANNEL_HORI = ARDRONE_VIDEO_CHANNEL_FIRST,			// Display the video provided by the horizontal camera
	ARDRONE_VIDEO_CHANNEL_VERT,			// Display the video provided by the vertical camera
	ARDRONE_VIDEO_CHANNEL_VERT_IN_HORI,	// Display the video provided by the horizontal camera and insert the video provided by the vertical camera 
	ARDRONE_VIDEO_CHANNEL_HORI_IN_VERT, // Display the video provided by the vertical camera and insert the video provided by the horiozontal camera
	ARDRONE_VIDEO_CHANNEL_LAST = ARDRONE_VIDEO_CHANNEL_HORI_IN_VERT,
	ARDRONE_VIDEO_CHANNEL_NEXT,			// Display the video provided by the next channel
} ARDRONE_VIDEO_CHANNEL;

typedef enum {
	ARDRONE_CAMERA_DETECTION_HORIZONTAL = 0,
	ARDRONE_CAMERA_DETECTION_VERTICAL,
	ARDRONE_CAMERA_VISION_DETECT,
	ARDRONE_CAMERA_NONE,
	ARDRONE_CAMERA_DETECTION_NUM
} ARDRONE_CAMERA_DETECTION_TYPE;

typedef enum {
	ARDRONE_LED_ANIMATION_BLINK_GREEN_RED,
	ARDRONE_LED_ANIMATION_BLINK_GREEN,
	ARDRONE_LED_ANIMATION_BLINK_RED,
	ARDRONE_LED_ANIMATION_SNAKE_GREEN_RED,
	ARDRONE_LED_ANIMATION_FIRE,
	ARDRONE_LED_ANIMATION_STANDARD,
	ARDRONE_LED_ANIMATION_RED,
	ARDRONE_LED_ANIMATION_GREEN,
	ARDRONE_LED_ANIMATION_RED_SNAKE,
	ARDRONE_LED_ANIMATION_BLANK,
	ARDRONE_LED_ANIMATION_RIGHT_MISSILE,
	ARDRONE_LED_ANIMATION_LEFT_MISSILE,
	ARDRONE_LED_ANIMATION_DOUBLE_MISSILE,
	ARDRONE_LED_ANIMATION_NUM
} ARDRONE_LED_ANIMATION;

typedef struct
{
	ARDRONE_CAMERA_DETECTION_TYPE camera_type;
	float tag_size;
} ARDRONE_CAMERA_DETECTION_PARAM;

typedef struct
{
	ARDRONE_LED_ANIMATION led_anim;
	float frequency;
	unsigned int duration;
} ARDRONE_LED_ANIMATION_PARAM;

/**
 * Define what a 3D vector is
 */
typedef struct
{
	float x;
	float y;
	float z;
}
ARDroneVector3D;

/**
 * Define a structure to collect drone's navigation data
 */
typedef struct
{
	/**
	 * Translation speed of the drone, in meters per second
	 */
	ARDroneVector3D linearVelocity;
	
	/**
	 * Rotation speed of the drone, in degré
	 */
	ARDroneVector3D angularPosition;
	
	/**
	 * Navdata video num frames to synchronized Navdata with video
	 */
	int navVideoNumFrames;
	
	/**
	 * Video num frames to synchronized Navdata with video
	 */
	int videoNumFrames;
	
	/**
	 * int indicates drone is in flying state (1 if is flying, 0 else)
	 */
	int flyingState;
	
	/**
	 * Camera detection type
	 */
	ARDRONE_CAMERA_DETECTION_TYPE detection_type;
}
ARDroneNavigationData;

/**
 * Define a structure to exchange an enemy data
 */
typedef struct
{
	/**
	 * Position of the enemy (between -1.0 and 1.0)
	 */
	ARDroneVector3D position;
	
	/**
	 * Size of the enemy (between 0.0 and 2.0)
	 */
	float height, width;	
}
ARDroneEnemyData;

/**
 * Define a structure to exchange camera parameters compute by detection
 */
typedef struct
{
	/**
	 * Rotation matrix of camera
	 */
	float rotation[3][3];
	
	/**
	 * Translation matrix of camera
	 */
	float translation[3];
	
	/**
	 * Index of tag detected
	 */
	int tag_index;
} 
ARDroneDetectionCamera;

/**
 * Define a structure to exchange camera parameters compute by drone
 */
typedef struct
{
	/**
	 * Rotation matrix of camera
	 */
	float rotation[3][3];
	
	/**
	 * Translation matrix of camera
	 */
	float translation[3];
} 
ARDroneCamera;

/**
 * Define a structure to exchange all enemies data
 */
typedef struct
{
	/**
	 * Number of enemies
	 */
	unsigned int count;
	
	/**
	 * Pointer to an array that contains the data structure of each enemy
	 */
	ARDroneEnemyData data[ARDRONE_MAX_ENEMIES];
}
ARDroneEnemiesData;

