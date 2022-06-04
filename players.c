#include "players.h"

const char *PLAYER_COLOUR_STRINGS[NUM_PLAYER_COLOURS] = {
	[COLOUR_WHITE] = "white",
	[COLOUR_BLACK] = "black",
};

const char *PIECE_SYMBOLS[NUM_PLAYER_COLOURS][NUM_CHESS_PIECES] = {
	[COLOUR_WHITE] =
	{
		[PIECE_NONE] = " ",
		[PIECE_PAWN] = "♙",
		[PIECE_KNIGHT] = "♘",
		[PIECE_ROOK] = "♖",
		[PIECE_BISHOP] = "♗",
		[PIECE_QUEEN] = "♕",
		[PIECE_KING] = "♔",
	},
	[COLOUR_BLACK] =
	{
		[PIECE_NONE] = " ",
		[PIECE_PAWN] = "♟️",
		[PIECE_KNIGHT] = "♞",
		[PIECE_ROOK] = "♜",
		[PIECE_BISHOP] = "♝",
		[PIECE_QUEEN] = "♛",
		[PIECE_KING] = "♚",
	},
};