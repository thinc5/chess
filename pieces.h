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
	NUM_CHESS_PIECES,
} ChessPiece;

extern const size_t STARTING_SIZES[NUM_CHESS_PIECES];
extern const char *CHESS_PIECE_STRINGS[NUM_CHESS_PIECES];

#endif
