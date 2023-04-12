#ifndef _MOVEMENT_H
#define _MOVEMENT_H

#include "movement_stats.h"
#include "board.h"

#include <stdlib.h>

#define MAX_POSSIBLE_MOVES 24

typedef struct {
	MovementType type;
	int target;
} PossibleMove;

MovementType none_movement_algorithm(
	Board board, int start, int target, size_t move_count, size_t check);
MovementType pawn_movement_algorithm(
	Board board, int start, int target, size_t move_count, size_t check);
MovementType knight_movement_algorithm(
	Board board, int start, int target, size_t move_count, size_t check);
MovementType rook_movement_algorithm(
	Board board, int start, int target, size_t move_count, size_t check);
MovementType bishop_movement_algorithm(
	Board board, int start, int target, size_t move_count, size_t check);
MovementType queen_movement_algorithm(
	Board board, int start, int target, size_t move_count, size_t check);
MovementType king_movement_algorithm(
	Board board, int start, int target, size_t move_count, size_t check);

extern const MovementType (*PIECE_MOVEMENT_ALGORITHM[NUM_CHESS_PIECES])(
	Board board, int start, int target, size_t move_count, size_t check);

size_t get_possible_moves_for_piece(Board board, int location,
				    PossibleMove possible_moves[MAX_POSSIBLE_MOVES], size_t move_count,
				    size_t check);


#endif
