/**************************************************
 * Extra tools for manipulating CvPoints
 * Author: Cooper Bills <csb88@gmail.com>
 * June 2010

 * Included Functions:

 * double   det2d(double a, double b, double c, double d) 

                //2D determinant
                //      |a b|
                //      |c d|
                                                        
 * CvPoint  findIntersectionPoint(CvPoint P1, CvPoint P2, CvPoint P3, CvPoint P4)
 
                //Finds the point of intersection of two (infinitely long) lines, 
                // defined by two sets of points:
                //P1 and P2 are points on first line
                //P3 and P4 are points on second line
                
 * int      dot(CvPoint v1, CvPoint v2) 
 
                //Dot Product
 
 * CvPoint  projVector(CvPoint vector, CvPoint target)
 
                //Projects 'vector' onto the vector 'target'
                //returns the resultant vector.
 *
 **************************************************/

#ifndef CVPOINTTOOLS_HPP
#define CVPOINTTOOLS_HPP

#include <opencv/highgui.h>

// 2D Determinant:
//      |a b|
//      |c d|
double det2d(double a, double b, double c, double d)
{
  return a*d - (b*c);
}

//Finds the intersection of two (infinitely long) lines, defined by two sets of points:
//P1 and P2 are points on first line
//P3 and P4 are points on second line
CvPoint findIntersectionPoint(CvPoint P1, CvPoint P2, CvPoint P3, CvPoint P4)
{
  //see: http://mathworld.wolfram.com/Line-LineIntersection.html

  double denom = det2d(P1.x - P2.x, P1.y - P2.y, P3.x - P4.x, P3.y - P4.y);

  if(denom == 0) return cvPoint(0, 0);

  double x = det2d( det2d(P1.x, P1.y, P2.x, P2.y),  P1.x - P2.x,
                    det2d(P3.x, P3.y, P4.x, P4.y),  P3.x - P4.x) / denom;

  double y = det2d( det2d(P1.x, P1.y, P2.x, P2.y),  P1.y - P2.y,
                    det2d(P3.x, P3.y, P4.x, P4.y),  P3.y - P4.y) / denom;

  return cvPoint((int)x, (int)y);
}

int dot(CvPoint v1, CvPoint v2)
{
  return (v1.x * v2.x) + (v1.y * v2.y);
}

CvPoint projVector(CvPoint vector, CvPoint target)
{
  //(dot(a, b)/dot(b, b))*b
  int numerator = dot(vector, target);
  int denominator = dot(target, target);
  if(denominator == 0) return cvPoint(0, 0);
  double distscale = (double)numerator/(double)denominator;
  return cvPoint((int)(target.x * distscale), (int)(target.y * distscale));
}


#endif
