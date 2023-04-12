#include "movement_stats.h"
#include "board.h"

#include <stdlib.h>

const char *MOVEMENT_TYPE_STRINGS[MOVEMENT_NUM_TYPES] = {
	"MOVEMENT_ILLEGAL", "MOVEMENT_NORMAL", "MOVEMENT_PIECE_CAPTURE",
	"MOVEMENT_PAWN_LONG_JUMP", "MOVEMENT_PAWN_EN_PASSANT",
	"MOVEMENT_PAWN_PROMOTION", "MOVEMENT_KING_CASTLE",
};

const char *MOVEMENT_DIRECTION_STRINGS[DIRECTION_NUM_DIRECTIONS] = {
	[DIRECTION_NONE] = "DIRECTION_NONE",
	[DIRECTION_NORTH] = "DIRECTION_NORTH",
	[DIRECTION_EAST] = "DIRECTION_EAST",
	[DIRECTION_SOUTH] = "DIRECTION_SOUTH",
	[DIRECTION_WEST] = "DIRECTION_WEST",
	[DIRECTION_NORTH_EAST] = "DIRECTION_NORTH_EAST",
	[DIRECTION_SOUTH_EAST] = "DIRECTION_SOUTH_EAST",
	[DIRECTION_SOUTH_WEST] = "DIRECTION_SOUTH_WEST",
	[DIRECTION_NORTH_WEST] = "DIRECTION_NORTH_WEST",
};

// static const int MOVE_DIRECTION_VALUES[DIRECTION_NUM_DIRECTIONS] = {
//     [DIRECTION_NONE]        = 0x00,
//     [DIRECTION_NORTH]       = 0x01,
//     [DIRECTION_EAST]        = 0x02,
//     [DIRECTION_SOUTH]       = 0x04,
//     [DIRECTION_WEST]        = 0x08,
//     [DIRECTION_NORTH_EAST]  = 0x01 | 0x02,
//     [DIRECTION_SOUTH_EAST]  = 0x04 | 0x02,
//     [DIRECTION_SOUTH_WEST]  = 0x04 | 0x08,
//     [DIRECTION_NORTH_WEST]  = 0x01 | 0x08
// };

EMovementDirection
get_movement_direction(size_t start_x, size_t start_y, size_t dest_x,
		       size_t dest_y)
{
	int x_dist = dest_x - start_x;
	int y_dist = dest_y - start_y;

	EMovementDirection x_dir = x_dist == 0 ? DIRECTION_NONE
	  : (x_dist > 0 ? DIRECTION_EAST : DIRECTION_WEST);
	EMovementDirection y_dir =
		y_dist == 0
	  ? DIRECTION_NONE
	  : (y_dist > 0 ? DIRECTION_NORTH : DIRECTION_SOUTH);

	// DEBUG_LOG("x: %s, y: %s\n", MOVEMENT_DIRECTION_STRINGS[x_dir], MOVEMENT_DIRECTION_STRINGS[y_dir]);

	// If we are only going in one direction we just return that direction.
	// DEBUG_LOG("d: %s\n", MOVEMENT_DIRECTION_STRINGS[x_dir | y_dir]);
	if (!x_dir != !y_dir) {
		// DEBUG_LOG("d: %s\n", MOVEMENT_DIRECTION_STRINGS[x_dir | y_dir]);
		return x_dir | y_dir;
	}

	// Two directions, find the diagonal.
	if (x_dir && y_dir) {
		if (x_dir == DIRECTION_EAST) {
			if (y_dir == DIRECTION_NORTH)
				return DIRECTION_NORTH_EAST;

			if (y_dir == DIRECTION_SOUTH)
				return DIRECTION_SOUTH_EAST;
		} else if (x_dir == DIRECTION_WEST) {
			if (y_dir == DIRECTION_NORTH)
				return DIRECTION_NORTH_WEST;

			if (y_dir == DIRECTION_SOUTH)
				return DIRECTION_SOUTH_WEST;
		}
	}

	// We have no direction.
	return DIRECTION_NONE;
}

MoveStats get_move_stats(size_t start, size_t target)
{
	int start_x = start % BOARD_SIZE;
	int start_y = start / BOARD_SIZE;
	int target_x = target % BOARD_SIZE;
	int target_y = target / BOARD_SIZE;
	return (MoveStats){
		       .start_x = start_x,
		       .target_x = target_x,
		       .dist_x = abs(target_x - start_x),
		       .start_y = start_y,
		       .target_y = target_y,
		       .dist_y = abs(target_y - start_y),
		       .direction = get_movement_direction(start_x, start_y,
							   target_x, target_y),
	};
}
