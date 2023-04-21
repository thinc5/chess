#ifndef _LOGIC_H
#define _LOGIC_H

#include "board.h"
#include "movement_stats.h"
#include "movement.h"

EMovementType process_movement(
	Board board, size_t move_count, int selected, int dest, size_t check);
size_t is_checkmate_for_player(
	Board board, EPlayerColour player, size_t move_count, size_t check);
size_t is_game_stalemate(Board board, EPlayerColour player, size_t move_count);
size_t is_game_over_for_player(Board board, Board next_board,
			       EPlayerColour player, size_t move_count,
			       size_t check);

#endif
