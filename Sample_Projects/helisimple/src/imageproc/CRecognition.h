/*
 * File name: CRecognition.h
 * Date:      2010
 * Author:   Tom Krajnik
 * the class finds blobs of particular color in the image
 * contains several blobfinding and classification methods,
 * because if was originally created for educational purposes
 *
 * for a potential user, important methods are learnPixel, resetColorMap and findSegment
 * each time the learnPixel() is called, a pixel color is added to a colortable
 * calling findSegment() tags pixels in the colortable, finds contiguous areas of the tagged pixels and returns data on the largest area
 * calling resetColorMap() clears the color tabe
 */

#ifndef __CRECOGNITION_H__
#define __CRECOGNITION_H__

#include "CRawImage.h"
#include <math.h>
#define MAX_SEGMENTS 10000

typedef struct{
	int x;
	int y;
	int size;
}SSegment;

typedef struct{
	int x;
	int y;
}SPixelPosition;

class CRecognition
{
public:
  CRecognition();
  ~CRecognition();

  //finds a largest segment of a particular color
  SPixelPosition findSegment(CRawImage* image);

  //
  void learnPixel(unsigned char* a);
  void increaseTolerance();
  void decreaseTolerance();
  void resetColorMap();

private:
  int tolerance;  
  float evaluatePixel1(unsigned char* a);
  float evaluatePixel2(unsigned char* a);
  float evaluatePixel3(unsigned char* a);
  int evaluatePixelFast(unsigned char *a);
  void rgbToHsv(unsigned char r, unsigned char  g, unsigned char b, unsigned int *h, unsigned char *s, unsigned char *v );
  SPixelPosition findMean(CRawImage* image);

  unsigned char learned[3];
  unsigned int learnedHue;
  unsigned char learnedSaturation,learnedValue;
  unsigned char colorArray[64*64*64];
  SSegment segmentArray[MAX_SEGMENTS];
  bool debug;
};

#endif

/* end of CRecognition.h */
