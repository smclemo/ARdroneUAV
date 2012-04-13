#ifndef CIMAGE_H
#define CIMAGE_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/**
@author Tom Krajnik
*/
//simple image class 
class CRawImage
{
	public:

		CRawImage(int w,int h);
		~CRawImage();

		//saves the image to a file with bmp format 
		void saveBmp(const char* name);

		//saves the image to a file with bmp format, 
		//calling this consecutivelly causes to save files with names 0000.bmp, 0001.bmp, 0002.bmp etc. 
		void saveBmp();

		//checks if there are images in the directory and sets the saving number in order to avoid overwriting them
		int  getSaveNumber();

		//loads a file in BMP format 
		//usefull only for bitmaps created with saveBmp(...) functions 
		bool loadBmp(const char* name);

		//plots a vertical and horizontal line crossing at x,y position 
		void plotLine(int x,int y);

		//plots a square in the center 
		void plotCenter();

		//image parameters
		int width;
		int height;
		int palette;
		int size;
		int bpp;

		//the image data in 24bit RGB bitmap format 
		unsigned char* data;
	private:
		//turns the image upside down, usefull for saving
		void swap();
		int numSaved;
};

#endif
