#ifndef _GAME_H
#define _GAME_H

#include "board.h"
#include "input.h"
#include "movement.h"

#include <stdbool.h>

typedef enum {
	GAME_MODE_INVALID,
	GAME_MODE_LOAD,
	GAME_MODE_LOCAL,
	GAME_MODE_REPLAY,
	GAME_MODE_HOST,
	GAME_MODE_JOIN,
	GAME_NUM_MODES
} EGameMode;

typedef enum {
	PLAYER_HUMAN,
	PLAYER_COMPUTER,
	PLAYER_NUM_TYPES,
} EPlayerType;

typedef struct {
	EGameMode prog_mode;
	EPlayerType player1;
	EPlayerType player2;
} ChessArgs;

typedef struct ChessGame {
	// The game board.
	Board board;
	// The phantom next board, the "perspective board".
	Board next_board;
	// This player's colour.
	EPlayerColour player;
	// Current player.
	EPlayerColour turn;
	// How many turns have happened in this game?
	size_t move_count;
	// How checkmated is this player? (How many ways are they in check.)
	size_t check;
	// Operation mode.
	EOperationMode mode;
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
	EGameMode mode, ChessGame *game, int connection_fd);
void toggle_player_turn(ChessGame *game);
bool select_piece(ChessGame *game);
bool select_piece_loc(ChessGame *game, int selected);
void clear_piece_selection(ChessGame *game);
bool move_piece(ChessGame *game);
bool move_piece_loc(ChessGame *game, int selected);

#endif
