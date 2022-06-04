#include "board.h"

#include <string.h>

#define CREATE_PIECE(_colour, _type)                                           \
	{ .type = _type, .colour = _colour, .moves = 0 }

#define FIRST_ROW(_colour)                                                     \
	CREATE_PIECE(_colour, PIECE_ROOK), CREATE_PIECE(_colour, PIECE_KNIGHT),      \
	CREATE_PIECE(_colour, PIECE_BISHOP), CREATE_PIECE(_colour, PIECE_QUEEN), \
	CREATE_PIECE(_colour, PIECE_KING), CREATE_PIECE(_colour, PIECE_BISHOP),  \
	CREATE_PIECE(_colour, PIECE_KNIGHT), CREATE_PIECE(_colour, PIECE_ROOK)

#define SECOND_ROW(_colour)                                                    \
	CREATE_PIECE(_colour, PIECE_PAWN), CREATE_PIECE(_colour, PIECE_PAWN),        \
	CREATE_PIECE(_colour, PIECE_PAWN), CREATE_PIECE(_colour, PIECE_PAWN),    \
	CREATE_PIECE(_colour, PIECE_PAWN), CREATE_PIECE(_colour, PIECE_PAWN),    \
	CREATE_PIECE(_colour, PIECE_PAWN), CREATE_PIECE(_colour, PIECE_PAWN)

#define EMPTY_ROW()                                                            \
	empty_space, empty_space, empty_space, empty_space, empty_space,             \
	empty_space, empty_space, empty_space

static const Board STARTING_BOARD = {
	FIRST_ROW(COLOUR_WHITE),
	SECOND_ROW(COLOUR_WHITE),
	EMPTY_ROW(),
	EMPTY_ROW(),
	EMPTY_ROW(),
	EMPTY_ROW(),
	SECOND_ROW(COLOUR_BLACK),
	FIRST_ROW(COLOUR_BLACK),
};

void new_board(Board dest)
{
	memcpy(dest, STARTING_BOARD, sizeof(Board));
}

void set_board(Board src, Board dest)
{
	memcpy(dest, src, sizeof(Board));
}
