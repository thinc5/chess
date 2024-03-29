#ifndef _MOVEMENT_STATS_H
#define _MOVEMENT_STATS_H

#include <stddef.h>

typedef enum {
	MOVEMENT_ILLEGAL,
	MOVEMENT_NORMAL,
	MOVEMENT_PIECE_CAPTURE,
	MOVEMENT_PAWN_LONG_JUMP,
	MOVEMENT_PAWN_EN_PASSANT,
	MOVEMENT_PAWN_PROMOTION,
	MOVEMENT_KING_CASTLE,
	MOVEMENT_NUM_TYPES
} EMovementType;

typedef enum {
	DIRECTION_NONE,
	DIRECTION_NORTH,
	DIRECTION_EAST,
	DIRECTION_SOUTH,
	DIRECTION_WEST,
	DIRECTION_NORTH_EAST,
	DIRECTION_SOUTH_EAST,
	DIRECTION_SOUTH_WEST,
	DIRECTION_NORTH_WEST,
	DIRECTION_NUM_DIRECTIONS
} EMovementDirection;

typedef struct {
	size_t start_x;
	size_t target_x;
	size_t dist_x;
	size_t start_y;
	size_t target_y;
	size_t dist_y;
	EMovementDirection direction;
} MoveStats;

extern const char *MOVEMENT_TYPE_STRINGS[MOVEMENT_NUM_TYPES];

EMovementDirection
get_movement_direction(size_t start_x, size_t start_y, size_t target_x,
		       size_t target_y);

MoveStats get_move_stats(size_t start, size_t target);

#endif
