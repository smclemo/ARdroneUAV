#include "coopertools.hpp"

//Random Helper functions
/*int abs(int i)
{
  return i<0 ? -i : i;
}*/

float abs(float i)
{
  return i<0 ? -i : i;
}

void drawX(IplImage *img, CvPoint p)
{
  CvScalar line_color = CV_RGB(255, 0, 0);
  cvLine( img, cvPoint(p.x + 3, p.y + 3), cvPoint(p.x - 3, p.y - 3), line_color, 2, CV_AA, 0 );
  cvLine( img, cvPoint(p.x - 3, p.y + 3), cvPoint(p.x + 3, p.y - 3), line_color, 2, CV_AA, 0 );
}

//calculates the length of overlap of two segments in 1D space
//p1, p2 are points on first line -- p3, p4 are line2
int distOverlap(int p1, int p2, int p3, int p4)
{
  int temp;
  //first, organize points
  if(p1 > p2) {temp = p1; p1 = p2; p2 = temp;}
  if(p3 > p4) {temp = p3; p3 = p4; p4 = temp;}
  
  //no overlap
  if(p2 < p3) return 0;
  if(p1 > p4) return 0;
  
  //full overlap
  if(p2 < p4 && p1 > p3) return p2 - p1;
  if(p3 > p1 && p4 < p2) return p4 - p3;
  
  //partal overlap
  if(p3 < p1) return p4 - p1;
  return p2 - p3;
}

//puts any degree value onto the -180 to 180 circle
float normDeg(float deg)
{
  //first, remove 360deg multiples
  if(((int)deg) / 360)
    deg = deg - (360.0*(((int)deg) / 360));
  
  //second, make negative if nessasary, and apply appropreate offsets
  if(deg <= -180.0)
    deg += 360.0;
  else if(deg > 180)
    deg -= 360.0;
  
  return deg;
}
  
//checks if valueDeg is inbetween startingDeg and endingDeg
int inDegSeg(float valueDeg, float startingDeg, float endingDeg)
{
  valueDeg = normDeg(valueDeg);
  startingDeg = normDeg(startingDeg);
  endingDeg = normDeg(endingDeg);
  
  if(startingDeg < endingDeg) //if non-wrapping segment
  {
    if(valueDeg >= startingDeg && valueDeg <= endingDeg)
      return 1;
    return 0;
  }
  else //if wrapping segment
  {
    if(valueDeg >= startingDeg)
      return 1;
    if(valueDeg <= startingDeg && valueDeg <= endingDeg)
      return 1;
    return 0;
  }
}

//returns degree offset between the two degrees (-180 to 180)
float degDiff(float deg1, float deg2)
{
  deg1 = normDeg(deg1);
  deg2 = normDeg(deg2);
  float diff1 = deg1 - deg2;
  return normDeg(diff1);
}
  
  
  
