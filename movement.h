#ifndef _MOVEMENT_H
#define _MOVEMENT_H

#include "movement_stats.h"
#include "board.h"

#include <stdlib.h>

#define MAX_POSSIBLE_MOVES 24

typedef struct {
	EMovementType type;
	int target;
} PossibleMove;

EMovementType none_movement_algorithm(
	Board board, int start, int target, size_t move_count, size_t check);
EMovementType pawn_movement_algorithm(
	Board board, int start, int target, size_t move_count, size_t check);
EMovementType knight_movement_algorithm(
	Board board, int start, int target, size_t move_count, size_t check);
EMovementType rook_movement_algorithm(
	Board board, int start, int target, size_t move_count, size_t check);
EMovementType bishop_movement_algorithm(
	Board board, int start, int target, size_t move_count, size_t check);
EMovementType queen_movement_algorithm(
	Board board, int start, int target, size_t move_count, size_t check);
EMovementType king_movement_algorithm(
	Board board, int start, int target, size_t move_count, size_t check);

extern const EMovementType (*PIECE_MOVEMENT_ALGORITHM[PIECE_NUM_PIECES])(
	Board board, int start, int target, size_t move_count, size_t check);

size_t get_possible_moves_for_piece(Board board, int location,
				    PossibleMove possible_moves[MAX_POSSIBLE_MOVES], size_t move_count,
				    size_t check);


#endif
