#include <stddef.h>

#include "pieces.h"

const size_t STARTING_SIZES[PIECE_NUM_PIECES] = {
	[PIECE_NONE] = 0, [PIECE_PAWN] = 8, [PIECE_KNIGHT] = 2,
	[PIECE_ROOK] = 2,
	[PIECE_BISHOP] = 2, [PIECE_QUEEN] = 1, [PIECE_KING] = 1,
};

const char *CHESS_PIECE_STRINGS[] = {
	[PIECE_NONE] = "none", [PIECE_PAWN] = "pawn", [PIECE_KNIGHT] = "knight",
	[PIECE_ROOK] = "rook", [PIECE_BISHOP] = "bishop",
	[PIECE_QUEEN] = "queen",
	[PIECE_KING] = "king",
};
