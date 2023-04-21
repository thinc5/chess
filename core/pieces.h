#ifndef _PIECES_H
#define _PIECES_H

#include <stddef.h>

// All the chess piece types.
typedef enum {
	PIECE_NONE,
	PIECE_PAWN,
	PIECE_KNIGHT,
	PIECE_ROOK,
	PIECE_BISHOP,
	PIECE_QUEEN,
	PIECE_KING,
	PIECE_NUM_PIECES,
} EChessPiece;

extern const size_t STARTING_SIZES[PIECE_NUM_PIECES];
extern const char *CHESS_PIECE_STRINGS[PIECE_NUM_PIECES];

#endif
