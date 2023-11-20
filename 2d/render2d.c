#include "config2d.h"
#include "../core/board.h"
#include "../core/movement.h"

#include <SDL2/SDL.h>

// From stack.
void draw_circle(SDL_Renderer *renderer, size_t centre_x, size_t centre_y,
		 size_t radius)
{
	const size_t diameter = (radius * 2);

	int x = (radius - 1);
	int y = 0;
	int tx = 1;
	int ty = 1;
	int error = (tx - diameter);

	while (x >= y) {
		SDL_RenderDrawPoint(renderer, centre_x + x, centre_y - y);
		SDL_RenderDrawPoint(renderer, centre_x + x, centre_y + y);
		SDL_RenderDrawPoint(renderer, centre_x - x, centre_y - y);
		SDL_RenderDrawPoint(renderer, centre_x - x, centre_y + y);
		SDL_RenderDrawPoint(renderer, centre_x + y, centre_y - x);
		SDL_RenderDrawPoint(renderer, centre_x + y, centre_y + x);
		SDL_RenderDrawPoint(renderer, centre_x - y, centre_y - x);
		SDL_RenderDrawPoint(renderer, centre_x - y, centre_y + x);

		if (error <= 0) {
			++y;
			error += ty;
			ty += 2;
		}

		if (error > 0) {
			--x;
			tx += 2;
			error += (tx - diameter);
		}
	}
}

SDL_Rect render_piece_loc(PlayPiece piece)
{
	size_t indexes[PIECE_NUM_PIECES] = {
		[PIECE_PAWN] = 5,
		[PIECE_BISHOP] = 4,
		[PIECE_KNIGHT] = 3,
		[PIECE_ROOK] = 2,
		[PIECE_KING] = 1,
		[PIECE_QUEEN] = 0
	};

	return (SDL_Rect) {
		       .x = indexes[piece.type] * PIECE_WIDTH,
		       .y = (piece.colour != COLOUR_WHITE ? 0 : PIECE_HEIGHT),
		       .w = PIECE_WIDTH,
		       .h = PIECE_HEIGHT
	};
}

void render_board(SDL_Renderer *renderer, SDL_Texture *texture, Board board,
		  int selected_piece, int num_possible_moves,
		  PossibleMove *possible_moves)
{
	static SDL_Rect tile = { .x = 0, .y = 0, .w = TILE_DIM, .h = TILE_DIM };
	for (size_t place = 0; place < BOARD_SIZE * BOARD_SIZE; place++) {
		size_t x = (place % BOARD_SIZE);
		size_t y = (place / BOARD_SIZE);
		size_t inverted_y = (BOARD_SIZE - 1 - (place / BOARD_SIZE));

		size_t even_x = (x + 1) % 2;
		size_t even_y = (y + 1) % 2;

		tile.x = (x * TILE_DIM) + X_OFFSET;
		tile.y = inverted_y * TILE_DIM;

		// We draw white squares when only one of x and y is even,
		// we draw black squares otherwise.
		if ((even_x && !even_y) || (!even_x && even_y)) {
			SDL_SetRenderDrawColor(renderer,
					       DISPLAY_COLOUR_BLACK_TILE);
		} else {
			SDL_SetRenderDrawColor(renderer,
					       DISPLAY_COLOUR_WHITE);
		}
		SDL_RenderFillRect(renderer, &tile);

		// Draw piece.
		PlayPiece piece = board[place];
		if (piece.type != PIECE_NONE) {
			SDL_Rect texture_loc = render_piece_loc(piece);
			SDL_RenderCopy(renderer, texture,
				       &texture_loc, &tile);
		}

		// Draw piece boarder.
		SDL_SetRenderDrawColor(renderer, DISPLAY_COLOUR_BLACK);
		SDL_RenderDrawRect(renderer, &tile);

		if (selected_piece != -1 && place == selected_piece) {
			SDL_SetRenderDrawColor(renderer,
					       DISPLAY_COLOUR_SELECTED);
			SDL_RenderDrawRect(renderer, &tile);
		}
	}

	// Draw all of our valid moves.
	for (size_t move_index = 0; move_index < num_possible_moves;
	     move_index++) {
		PossibleMove *move = &possible_moves[move_index];
		size_t x = (move->target % BOARD_SIZE);
		size_t y = BOARD_SIZE - 1 - (move->target / BOARD_SIZE);
		tile.x = (x * TILE_DIM) + X_OFFSET;
		tile.y = y * TILE_DIM;
		switch (move->type) {
		case MOVEMENT_PIECE_CAPTURE:
			SDL_SetRenderDrawColor(renderer,
					       DISPLAY_COLOUR_SELECTED);
			draw_circle(renderer, tile.x + (TILE_DIM / 2),
				    tile.y + (TILE_DIM / 2), TILE_DIM / 4);
			break;
		default:
			SDL_SetRenderDrawColor(renderer,
					       DISPLAY_COLOUR_MOVE);
			draw_circle(renderer, tile.x + (TILE_DIM / 2),
				    tile.y + (TILE_DIM / 2), TILE_DIM / 4);
			break;
		}
	}

	static const SDL_Rect boarder =
	{ .x = X_OFFSET, .y = 0, .w = TILE_DIM * BOARD_SIZE,
	  .h = TILE_DIM * BOARD_SIZE };
	SDL_SetRenderDrawColor(renderer, DISPLAY_COLOUR_BLACK);
	SDL_RenderDrawRect(renderer, &boarder);
}
