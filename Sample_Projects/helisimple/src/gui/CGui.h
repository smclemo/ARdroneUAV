#ifndef __CGUI_H__
#define __CGUI_H__

#include "CRawImage.h"
#include <math.h>
#include <SDL/SDL.h>

class CGui
{
public:
  CGui(int w,int h);
  ~CGui();

  void drawImage(CRawImage* image);
  void update();

private:
  SDL_Surface *screen;
};

#endif

/* end of CGui.h */
