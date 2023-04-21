#ifndef PGN_H
#define PGN_H

#include "replay.h"
#include "game.h"

#include <stdlib.h>

EReplayInput read_next_move_pgn(FILE *file, ChessGame *game, char *input_buffer,
				size_t *input_pointer);

#endif
