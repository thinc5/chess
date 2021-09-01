#ifndef _GAME_H
#define _GAME_H

#include "board.h"

#include "input.h"
#include "movement.h"

typedef struct ChessGame {
    // The game board.
    Board board;
    // The phantom next board, the "perspective board".
    Board next_board;
    // Current player.
    PlayerColour turn;
    // How many turns have happened in this game?
    size_t move_count;
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

#endif
