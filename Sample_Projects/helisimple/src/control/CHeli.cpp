#include "CHeli.h"

float saturation(float a,float b)
{
	if (a > b) return b;
	if (a < -b) return -b;
	return a;
}

CHeli::CHeli()
{
	landed = true;
	appInit();
	imageWidth = 320;
	imageHeight = 240;
}

CHeli::~CHeli()
{
	appDeinit();
}

void CHeli::takeoff()
{
	if (landed){
		usleep(100000);
		at_ui_reset();
		usleep(200000);
		at_trim();
		usleep(200000);
		fprintf(stdout,"Taking off");
		at_ui_pad_start_pressed();
		usleep(100000);
		at_comwdg();
		landed = false;
	}
}

void CHeli::switchCamera(int cam){
	at_zap(cam);
}

void CHeli::land()
{
	if (landed ==false){
		usleep(100000);
		at_ui_pad_start_pressed();
		usleep(100000);
		landed = true;
	}
}

void CHeli::close()
{
	appDeinit();
}

int CHeli::renewImage(CRawImage* image)
{
	unsigned char *picbuf = (unsigned char*)picture_buf;
	//  width = picture_width;
	//  height = picture_height;
	if (picture_width == 320){
		for (int i= 0;i<imageWidth*imageHeight;i++){
			image->data[3*i] = (picbuf[2*i+1]&0xf8);	
			image->data[3*i+1] = ((picbuf[2*i+1]&0x07)*32)+((picbuf[2*i]&0xe0)/8);	
			image->data[3*i+2] = (picbuf[2*i]&0x1f)*8;
		}
	}else{
		memset(image->data,0,imageWidth*imageHeight*3);
		for (int w= 0;w<picture_width;w++){
			for (int h= 0;h<picture_height;h++){
				image->data[3*((h+58)*imageWidth+w+78)+0] = (picbuf[2*(h*imageWidth+w)+1]&0xf8);	
				image->data[3*((h+58)*imageWidth+w+78)+1] = ((picbuf[2*(h*imageWidth+w)+1]&0x07)*32)+((picbuf[2*(h*imageWidth+w)]&0xe0)/8);	
				image->data[3*((h+58)*imageWidth+w+78)+2] = (picbuf[2*(h*imageWidth+w)]&0x1f)*8;
			}
		}
	}
	return 0;
}

void CHeli::setAngles(float ipitch, float iroll,float iyaw,float iheight)
{
	int32_t yaw,pitch,roll,height;

	yaw = saturation(iyaw,33000);
	roll = saturation(iroll,33000);
	pitch = saturation(ipitch,33000);
	height = saturation(iheight,33000);

//	fprintf(stdout,"Angle request: %d %d %d %d ",pitch,roll,height,yaw);
	at_set_radiogp_input(roll,pitch,height,yaw);
}	
