#ifndef _RENDER_3D_H
#define _RENDER_3D_H

#include "triangle3d.h"

#include <SDL2/SDL.h>

void draw_triangle_frame(SDL_Renderer *renderer, Triangle3D tri);

void draw_triangle_fill(SDL_Renderer *renderer, Triangle3D tri);

#endif
