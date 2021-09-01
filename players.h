#ifndef _PLAYERS_H
#define _PLAYERS_H

#include "pieces.h"

typedef enum {
    COLOUR_WHITE,
    COLOUR_BLACK,
    NUM_PLAYER_COLOURS,
} PlayerColour;

extern const char *PLAYER_COLOUR_STRINGS[NUM_PLAYER_COLOURS];
extern const char *PIECE_SYMBOLS[NUM_PLAYER_COLOURS][NUM_CHESS_PIECES];

#endif
