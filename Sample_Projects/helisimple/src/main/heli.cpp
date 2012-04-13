/*
 * A simple 'getting started' interface to the ARDrone, v0.2 
 * author: Tom Krajnik
 * The code is straightforward,
 * check out the CHeli class and main() to see 
 */
#include <stdlib.h>
#include "CGui.h"
#include "CRecognition.h"
#include "CHeli.h"

int i = 0;
int numSaved = 0;
bool stop = false;
CGui* gui;
CRawImage *image;
SDL_Event event;
CRecognition *recognition;
SPixelPosition pos;
bool move = false;
Uint8 lastKeys[10000];
int keyNumber = 10000;
Uint8 *keys = NULL;
CHeli *heli;
float pitch,roll,yaw,height;
SDL_Joystick* joystick;
TJoyState joy,lastJoy;

// processing input from a joystick
void processJoystick(bool print = false)
{	
	SDL_JoystickUpdate();
	if (print) printf("Joystick ");
	for (int i = 0;i<6;i++){
		joy.axis[i] = SDL_JoystickGetAxis (joystick, i);
		if (print) printf("%i ",joy.axis[i]);
		if (fabs(joy.axis[i]) < 20) joy.axis[i] = 0;
	}
	for (int i = 0;i<11;i++){
		 joy.buttons[i+1] =  SDL_JoystickGetButton (joystick,i);
	}
	if (print) printf("\n");
	roll	= joy.axis[0];	
	pitch 	= joy.axis[1];
	yaw 	= joy.axis[2];
	height 	= joy.axis[3];

	if (joy.buttons[7] && lastJoy.buttons[7] == false) heli->takeoff();
	if (joy.buttons[5] && lastJoy.buttons[5] == false) heli->land();
	for (int i = 1;i<5;i++){
		if (joy.buttons[i] && lastJoy.buttons[i]==false) heli->switchCamera(i-1);
	}
	lastJoy = joy;
}

// processing keyboard and mouse events 
void processKeys()
{
	keys = SDL_GetKeyState(NULL);
	//clicking with a mouse on a pixel in the image causes the segmentation to consider the color an object to segment 
	while (SDL_PollEvent(&event)){
		if (event.type == SDL_MOUSEBUTTONDOWN) recognition->learnPixel(&image->data[3*(image->width*event.motion.y + event.motion.x)]);
	}
	//causes the segmentation to forget everything it has learned
	if (keys[SDLK_r]) recognition->resetColorMap();
	keys = SDL_GetKeyState(&keyNumber);
	if (keys[SDLK_ESCAPE]) stop = true;
	if (keys[SDLK_RETURN])image->saveBmp();

	//moves the drone
	if (keys[SDLK_KP7])  yaw = -20000.0;
	if (keys[SDLK_KP9])  yaw = 20000.0;
	if (keys[SDLK_KP4])  roll = -20000.0;
	if (keys[SDLK_KP6])  roll = 20000.0;
	if (keys[SDLK_KP8])  pitch = -20000.0;
	if (keys[SDLK_KP2])  pitch = 20000.0;
	if (keys[SDLK_KP_PLUS])  height = 20000.0;
	if (keys[SDLK_KP_MINUS])  height = -20000.0;

	//changes camera
	if (keys[SDLK_z]) heli->switchCamera(0);
	if (keys[SDLK_x]) heli->switchCamera(1);
	if (keys[SDLK_c]) heli->switchCamera(2);
	if (keys[SDLK_v]) heli->switchCamera(3);

	//this is obvoius
	if (keys[SDLK_q]) heli->takeoff();
	if (keys[SDLK_a]) heli->land();

	memcpy(lastKeys,keys,keyNumber);
}

//main loop
int main(int argc,char* argv[])
{
	//establishing connection with the quadcopter
	heli = new CHeli();
	//initializing GUI and joystick
	gui = new CGui(320,240);
	joystick = SDL_JoystickOpen(1);
	fprintf(stdout,"Joystick with %i axes, %i buttons and %i hats initialized.\n",SDL_JoystickNumAxes(joystick),SDL_JoystickNumButtons(joystick),SDL_JoystickNumHats(joystick));

	//this class holds the image from the drone	
	image = new CRawImage(320,240);
	//this class can segment the image	
	recognition = new CRecognition();
	image->getSaveNumber();
	
	while (stop == false){
		//prints the drone telemetric data, helidata struct contains drone angles, speeds and battery status 
		fprintf(stdout,"Angles %.2lf %.2lf %.2lf ",helidata.phi,helidata.psi,helidata.theta);
		fprintf(stdout,"Speeds %.2lf %.2lf %.2lf ",helidata.vx,helidata.vy,helidata.vz);
		fprintf(stdout,"Battery %.0lf ",helidata.battery);
		fprintf(stdout,"Largest blob %i %i\n",pos.x,pos.y);

		//image is captured
		heli->renewImage(image);


		//finding a blob in the image 
		pos = recognition->findSegment(image);

		//turns the drone towards the colored blob 
		//yaw = 100*(pos.x-160); //uncomment to make the drone to turn towards a colored target

		//getting input from the user
		processJoystick();
		processKeys();

		//setting the drone angles 
		heli->setAngles(pitch,roll,yaw,height);
		pitch=roll=yaw=height=0.0;

		//drawing the image, the cross etc.
		if (move==false || i%1 ==0){
			image->plotLine(pos.x,pos.y);
			image->plotCenter();
			gui->drawImage(image);
			gui->update();	
		}
		i++;
		usleep(20000);
	}
	delete recognition;
	delete heli;
	delete image;
	delete gui;
	return 0;
}

