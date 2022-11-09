#ifndef _GAME_H
#define _GAME_H

#include <stdbool.h>

#include "board.h"
#include "input.h"
#include "movement.h"

typedef enum {
	MODE_INVALID,
	MODE_LOCAL,
	MODE_REPLAY,
	MODE_HOST,
	MODE_JOIN,
	NUM_PROGRAM_MODES
} ProgramMode;

typedef enum {
	PLAYER_HUMAN,
	PLAYER_COMPUTER,
	NUM_PLAYER_TYPES,
} PlayerType;

typedef struct {
	ProgramMode prog_mode;
	PlayerType player1;
	PlayerType player2;
} ChessArgs;

typedef struct ChessGame {
	// The game board.
	Board board;
	// The phantom next board, the "perspective board".
	Board next_board;
	// This player's colour.
	PlayerColour player;
	// Current player.
	PlayerColour turn;
	// How many turns have happened in this game?
	size_t move_count;
	// How checkmated is this player? (How many ways are they in check.)
	size_t check;
	// Operation mode.
	OperationMode mode;
	// Selected piece, -1 for none.
	int selected_piece;
	// Information on the selected piece.
	size_t num_possible_moves;
	PossibleMove possible_moves[MAX_POSSIBLE_MOVES];
	// Input parsing.
	size_t input_pointer;
	char input_buffer[INPUT_BUFFER_SIZE];
} ChessGame;

void init_chess_game(ChessGame *game);
void play_chess(ChessGame *game);
void play_chess_networked(
	ProgramMode mode, ChessGame *game, int connection_fd);
bool move_piece(ChessGame *game, Command command);

#endif
