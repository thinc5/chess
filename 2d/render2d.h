#ifndef _RENDER_2D_H
#define _RENDER_2D_H

#include "../core/board.h"

#include <SDL2/SDL.h>

void draw_circle(SDL_Renderer *renderer, size_t centre_x, size_t centre_y,
		 size_t radius);

SDL_Rect render_piece_loc(PlayPiece piece);

void render_board(SDL_Renderer *renderer, SDL_Texture *texture, Board board,
		  int selected_piece, int num_possible_moves,
		  PossibleMove *possible_moves);

#endif
