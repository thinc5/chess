#ifndef _PLAYERS_H
#define _PLAYERS_H

#include "pieces.h"

typedef enum {
	COLOUR_WHITE,
	COLOUR_BLACK,
	PLAYER_NUM_COLOURS,
} EPlayerColour;

extern const char *PLAYER_COLOUR_STRINGS[PLAYER_NUM_COLOURS];
extern const char *PIECE_SYMBOLS[PLAYER_NUM_COLOURS][PIECE_NUM_PIECES];

#endif
