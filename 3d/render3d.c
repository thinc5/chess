#include "render3d.h"

#include <SDL2/SDL.h>

void draw_triangle_frame(SDL_Renderer *renderer, Triangle3D tri)
{
	SDL_RenderDrawLine(renderer, tri[0][VECTOR_3D_X], tri[0][VECTOR_3D_Y],
			   tri[1][VECTOR_3D_X], tri[1][VECTOR_3D_Y]);
	SDL_RenderDrawLine(renderer, tri[1][VECTOR_3D_X], tri[1][VECTOR_3D_Y],
			   tri[2][VECTOR_3D_X], tri[2][VECTOR_3D_Y]);
	SDL_RenderDrawLine(renderer, tri[2][VECTOR_3D_X], tri[2][VECTOR_3D_Y],
			   tri[0][VECTOR_3D_X], tri[0][VECTOR_3D_Y]);
}

void draw_triangle_fill(SDL_Renderer *renderer, Triangle3D tri)
{
}
