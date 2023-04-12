#ifndef _REPLAY_H
#define _REPLAY_H

#include <stdio.h>

#include "game.h"

typedef enum {
	REPLAY_INPUT_INVALID,
	REPLAY_INPUT_COMMENT,
	REPLAY_INPUT_COORDS,
	REPLAY_INPUT_DRAW,
	REPLAY_INPUT_WIN_WHITE,
	REPLAY_INPUT_WIN_BLACK,
	REPLAY_INPUT_REPLAY_OVER,
	REPLAY_INPUT_EOF,
	REPLAY_INPUT_NUM_TYPES
} REPLAY_INPUT;

extern char *REPLAY_INPUT_STRINGS[REPLAY_INPUT_NUM_TYPES];

int read_char_from_file(FILE *file, char *input_buffer,
			size_t *input_pointer);

int read_line_from_file(FILE *file, char *input_buffer,
			size_t *input_pointer);

void replay_chess(ChessGame *game, const char *filepath);

#endif
