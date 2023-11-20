#include "config2d.h"

#include "../core/board.h"

int screen_to_piece_loc(int x, int y)
{
	int x_loc = (x - X_OFFSET) / TILE_DIM;
	int y_loc = BOARD_SIZE - 1 - (y / TILE_DIM);
	return x_loc + (y_loc * BOARD_SIZE);
}
