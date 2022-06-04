#ifndef _BOARD_H
#define _BOARD_H

#include "players.h"

#define BOARD_SIZE 8

typedef struct {
	ChessPiece type;
	PlayerColour colour;
	size_t moves;
	size_t last_move;
} PlayPiece;

typedef PlayPiece Board[BOARD_SIZE * BOARD_SIZE];

static const PlayPiece empty_space = {
	.type = PIECE_NONE,
	.colour = COLOUR_WHITE,
	.moves = 0,
	.last_move = 0,
};

void new_board(Board dest);

void set_board(Board src, Board dest);

#endif
