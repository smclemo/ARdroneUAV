#include "CGui.h"

#define THICK_CROSS

CGui::CGui(int width,int height)
{
  SDL_Init(SDL_INIT_VIDEO|SDL_INIT_JOYSTICK);
  screen = NULL;
  screen = SDL_SetVideoMode(width,height,24,SDL_SWSURFACE); 
  if (screen == NULL)fprintf(stderr,"Couldn't set SDL video mode: %s\r\n",SDL_GetError());
  SDL_WM_SetCaption("Simple AR-drone vision system","Simple AR-drone vision system");
}

CGui::~CGui()
{
}

void CGui::drawImage(CRawImage* image)
{
  int result = 0;
  SDL_Surface *imageSDL = SDL_CreateRGBSurfaceFrom(image->data,image->width,image->height,image->bpp*8,image->bpp*image->width,0x000000ff,0x0000ff00,0x00ff0000,0x00000000);
  if (imageSDL != NULL && SDL_BlitSurface(imageSDL, NULL, screen, NULL)==0) result = 0;
  SDL_FreeSurface(imageSDL);
}

void CGui::update()
{
  SDL_UpdateRect(screen,0,0,0,0);
}
