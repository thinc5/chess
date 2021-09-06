#ifndef _LOGIC_H
#define _LOGIC_H

#include "movement.h"

#include "board.h"

MovementType process_movement(Board board, size_t move_count, int selected, int dest, size_t check);
size_t is_checkmate_for_player(Board board, PlayerColour player, size_t move_count, size_t check);
size_t is_game_over_for_player(Board board, Board next_board, PlayerColour player, size_t move_count, size_t check);

#endif
