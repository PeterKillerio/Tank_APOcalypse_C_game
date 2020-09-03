#include <assert.h>

#include <SDL2/SDL.h>

#include <SDL2/SDL_image.h>

#include "xwin_sdl.h"

static SDL_Window *win = NULL;

int xwin_init(int w, int h)
{
   int r;
   r = SDL_Init(SDL_INIT_VIDEO);
   assert(win == NULL);
   win = SDL_CreateWindow("PRG Semester Project", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, w, h, SDL_WINDOW_SHOWN);
   assert(win != NULL);
   SDL_SetWindowTitle(win, "Tank APOcalypse");
   //SDL_Surface *surface = SDL_CreateRGBSurfaceFrom(icon_32x32_bits, 32, 32, 24, 32*3, 0xff, 0xff00, 0xff0000, 0x0000);
   //SDL_SetWindowIcon(win, surface);
   //SDL_FreeSurface(surface);
   return r;
}

void xwin_close()
{
   assert(win != NULL);
   SDL_DestroyWindow(win);
   SDL_Quit();
   win = NULL;
}

void xwin_redraw(int w, int h, unsigned char *img)
{
   assert(img && win);
   SDL_Surface *scr = SDL_GetWindowSurface(win);
   for(int y = 0; y < scr->h; ++y) {
      for(int x = 0; x < scr->w; ++x) {
         const int idx = (y * scr->w + x) * scr->format->BytesPerPixel;
         Uint8 *px = (Uint8*)scr->pixels + idx;
         *(px + scr->format->Rshift / 8) = *(img++);
         *(px + scr->format->Gshift / 8) = *(img++);
         *(px + scr->format->Bshift / 8) = *(img++);
      }
   }
   SDL_UpdateWindowSurface(win);
}

void xwin_resize(int w, int h)
{
	xwin_close();
	xwin_init(w, h);
}

void xwin_poll_events(void)
{
   SDL_Event event;
   while (SDL_PollEvent(&event));
}
/* end of xwin_sdl.c */
