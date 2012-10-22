#include <stdlib.h>
#include <time.h>
#include <iostream>
#include <fstream>

#include "pointset.h"
#include "fast.h"
#include <pthread.h>
#include "neon.h"
#include "halfsize_neon.h"
#include "commands.h"
#include "camera_coord_lookup.h"

#define DEBUG_TAG "Tom-Native-stuff"

#include "comms/ClientSocket.h"
#include "comms/Socket.h"

#include "Tom_fast9.h"
#include "Tom_fast_score.h"

#include "cvd_lite/image.h"
#include "util/container.h"
#include "util/bounded_container.h"
#include "cvd_lite/camera.h"
#include "descriptor.h"
#include "nonmax.h"

#include "matcher.h"
#include "native.h"

#include "../video.h"
#include "../maths.h"
#include "../util.h"

#include <fcntl.h>
#include <stdio.h>
#include <string.h>

using namespace CVD;

pthread_mutex_t the_mutex = PTHREAD_MUTEX_INITIALIZER;;
pthread_t native_thread;

/* return current time in milliseconds 
static double
now_ms(void)
{
        struct timespec res;
        clock_gettime(CLOCK_THREAD_CPUTIME_ID, &res);
        return 1000.0*res.tv_sec + ((double)res.tv_nsec)/1000000;
}*/

double time_last_good = -2.0;
FILE *log_file;
//FILE *log_file2;

PointSet corners;
Commands command;
pose last_good_location;

Container<ImageRef> the_corners;
Container<int> the_corner_scores;
Bounded_Container<ImageRef> the_max_corners;
Container<Descriptor> the_max_corner_descriptors;
Matcher the_matcher;
Container<int> the_corner_matches;

Image<TooN::Vector<2, float> > drone_xy_lookup;
Camera::Quintic drone_camera_model;

std::string serverip = "192.168.1.4";//"192.168.1.100";//"192.168.43.19";//"192.168.1.234";
int const serverport = 10000;
ClientSocket client_socket;
sockaddr_storage server_addr;
socklen_t server_addr_len = sizeof(*(client_socket.m_addr));

static const unsigned int WIDTH = 1280;
static const unsigned int HEIGHT = 720;

double start_time;
unsigned int frames_processed = 0;

/*char *occlusion_mask;
static char const bit0 = 1;
static char const bit1 = 1 << 1;
static char const bit2 = 1 << 2;
static char const bit3 = 1 << 3;
static char const bit4 = 1 << 4;
static char const bit5 = 1 << 5;
static char const bit6 = 1 << 6;
static char const bit7 = 1 << 7;*/


unsigned int counter = 0, ind = 0;
static const unsigned int BUF_SIZE = 3;
float x_buffer[BUF_SIZE], y_buffer[BUF_SIZE], z_buffer[BUF_SIZE], yaw_buffer[BUF_SIZE];
bool filter_pose(struct pose *location)
{
	if(abs(location->x) > 4 || abs(location->y) > 1.5 || abs(location->z) > 4 || abs(location->yaw) > PI/3)
		return false;
	if(location->x == 0 || location->y == 0 || location->z == 0 || location->yaw == 0)
		return false;
		
	x_buffer[ind] = location->x;
	y_buffer[ind] = location->y;
	z_buffer[ind] = location->z;
	yaw_buffer[ind] = location->yaw;
	ind = (ind+1)%BUF_SIZE;
	
	if(counter++ < BUF_SIZE)
		return false;
	
	location->x = util_median(x_buffer, BUF_SIZE);
	location->y = util_median(y_buffer, BUF_SIZE);
	location->z = util_median(z_buffer, BUF_SIZE);
	location->yaw = util_median(yaw_buffer, BUF_SIZE);
	
	return true;
}


void* native_thread_main(void* data)
{
	CVD::Image<unsigned char> frame_grey = CVD::Image<unsigned char>(CVD::ImageRef(WIDTH, HEIGHT));
	pose received_location;
	
	received_location.x = 0.0;
	received_location.y = 0.0;
	received_location.z = 0.0;
	received_location.yaw = 0.0;
	
	start_time = util_timestamp();
	while(true)
	{
		//double t1 = util_timestamp();
		get_frame_grey(frame_grey.data());

		corners.build_from_image(frame_grey, drone_xy_lookup, true);//use_rhips

		client_socket.sendto(corners, server_addr, server_addr_len);
		//fprintf(log_file2,"%f,%d\n", (util_timestamp()-t1), corners.size());
		
		pthread_yield();

		if(client_socket.recvfrom(command, server_addr, server_addr_len))
		{
			received_location.x = command.packet.transform[0];
			received_location.y = command.packet.transform[1];
			received_location.z = command.packet.transform[2];
			received_location.yaw = command.packet.transform[4];
			
			if(filter_pose(&received_location))
			{
				pthread_mutex_lock(&the_mutex);
				
				last_good_location.x = received_location.x;
				last_good_location.y = received_location.y;
				last_good_location.z = received_location.z;
				last_good_location.yaw = received_location.yaw;		
				
				time_last_good = util_timestamp();
				
				pthread_mutex_unlock(&the_mutex);
				
				fprintf(log_file,"%f,%f,%f,%f,%f\n"
					,time_last_good   	// timestamp in sec  
					,received_location.x   	// x loction            
					,received_location.y  	// y location    
					,received_location.z		// Z location
					,received_location.yaw);
			}
		}

		frames_processed++;
		//printf("found %d corners\n", corners.size());
		//printf("estimated pose (x,y,z,yaw): %f, %f, %f, %f\n", received_location.x, received_location.y, received_location.z, received_location.yaw);   
		
		while(!video_frame_ready())
			usleep(50);
	}
}

void initnative() 
{
	corners.erase();

	the_corners.set_max_size(16);
	the_corner_scores.set_max_size(16);
	the_max_corners.set_max_size(16);
	the_max_corner_descriptors.set_max_size(16);
	
	drone_xy_lookup = Image<TooN::Vector<2, float> >(ImageRef(WIDTH, HEIGHT));

	std::ifstream is;
	is.open("dronecamparameters.txt");
	drone_camera_model.load(is);
	is.close();
	
	//TooN::Vector<6> cam_params = drone_camera_model.get_parameters();
	//printf("cam params: %f %f %f %f %f %f\n", cam_params[0], cam_params[1], cam_params[2], cam_params[3], cam_params[4], cam_params[5]);
	generate_xy_lookup(drone_camera_model, drone_xy_lookup);

	client_socket.create(serverip, serverport);
	
	log_file = fopen("udp_logX.csv", "wb");
	if(log_file==NULL)
	{
		printf("File 2 open failed\n");
	}
	fprintf(log_file,"Time,X (m),Y (m),Z (m),Yaw (rad)\n");
	
	/*log_file2 = fopen("udp_log2.csv", "wb");
	if(log_file2==NULL)
	{
		printf("File 3 open failed\n");
	}
	fprintf(log_file2,"Time,Corners\n");*/
	
	//init front camera
	while(video_init((char*) "/dev/video1", WIDTH, HEIGHT, 30))
	{
		printf("Camera initialisation failed. Retry in 2 seconds...\n");
		sleep(2);
	}
	printf("video_init completed\n");
	
	sleep(2);
	
	int rc = pthread_create(&native_thread, NULL, native_thread_main, NULL);
	if (rc)
		printf("ctl_Init: Return code from pthread_create(native_thread) is %d\n", rc);
	
	last_good_location.x = 0.0;	
	last_good_location.y = 0.0;
	last_good_location.z = 0.0;
	last_good_location.yaw = 0.0;
	
	printf("native_init completed\n");
}

void uninitnative() 
{	
	pthread_cancel(native_thread);
	
	printf("\nFPS: %f\n", (double)frames_processed/(util_timestamp()-start_time));
	
	video_close();
	
	if(fclose(log_file) != 0)
		printf("File 2 not closed!");
		
	//if(fclose(log_file2) != 0)
		//printf("File 3 not closed!");
}

bool get_pose(pose* location) 
{	
	pthread_mutex_lock(&the_mutex);
	
	if(util_timestamp() - time_last_good > 0.3) 	// last good pose older than 0.3 second
	{
		pthread_mutex_unlock(&the_mutex);
		return false;
	}
	
	location->x = last_good_location.x;
	location->y = last_good_location.y;
	location->z = last_good_location.z;
	location->yaw = last_good_location.yaw;
	
	pthread_mutex_unlock(&the_mutex);
	
	return true;
}

void saveKeypoints(Image<unsigned char>& imageToSave)
{
	for(int i=0; i<corners.size(); i++)
	{
		TooN::Vector<2> undistorted_coords = drone_camera_model.linearproject(corners[i].cam_coords);
		if(imageToSave.in_image(ImageRef((int) undistorted_coords[0], (int) undistorted_coords[1])))
			drawBox(imageToSave.data(), undistorted_coords[0], undistorted_coords[1], 6);
	}

	printf("saving keypoints\n");
	
	FILE *file_fd;
	file_fd = fopen("keypoints12.yuv", "wb");
	const void *p;
	p = (const void*) imageToSave.data();
	fwrite(p, WIDTH*HEIGHT, 1, file_fd);
	fclose(file_fd);
}

void undistort(Image<unsigned char>& distortedImage) 
{
	ImageRef frame_size = ImageRef(WIDTH, HEIGHT);
	Image<unsigned char> undistortedImage = Image<unsigned char>((unsigned char*)malloc(WIDTH*HEIGHT*sizeof(char)), frame_size, WIDTH);
	printf("starting\n");
	
	TooN::Vector<6> cam_params = drone_camera_model.get_parameters();
	
	for(int x=0; x<WIDTH; x++)
	{
		for(int y=0; y<HEIGHT; y++)
		{
			ImageRef distorted_coords_ref = ImageRef(x, y);

			TooN::Vector<2> undistorted_coords = drone_camera_model.linearproject(drone_xy_lookup[distorted_coords_ref]);
			ImageRef undistorted_coords_ref = ImageRef((int) undistorted_coords[0], (int) undistorted_coords[1]);
			
			//printf("in: x=%d, y=%d out: x=%d, y=%d\n", distorted_coords_ref[0], distorted_coords_ref[1], undistorted_coords_ref[0], undistorted_coords_ref[1]);
			if(undistortedImage.in_image(undistorted_coords_ref))
				undistortedImage[undistorted_coords_ref] = distortedImage[distorted_coords_ref];
		}
	}
	
	printf("saving distortion\n");
	
	FILE *file_fd;
	file_fd = fopen("distorted12.yuv", "wb");
	const void *p;
	p = (const void*) distortedImage.data();
	fwrite(p, WIDTH*HEIGHT, 1, file_fd);
	fclose(file_fd);
	
	file_fd = fopen("undistorted12.yuv", "wb");
	p = (const void*) undistortedImage.data();
	fwrite(p, WIDTH*HEIGHT, 1, file_fd);
	fclose(file_fd);
	
	distortedImage = undistortedImage;
}
