#ifndef _MOVEMENT_H
#define _MOVEMENT_H

#include <stdlib.h>

#include "board.h"

#define MAX_POSSIBLE_MOVES 24

typedef enum {
	MOVEMENT_ILLEGAL,
	MOVEMENT_NORMAL,
	MOVEMENT_PIECE_TAKE,
	MOVEMENT_PAWN_LONG_JUMP,
	MOVEMENT_PAWN_EN_PASSANT,
	MOVEMENT_PAWN_PROMOTION,
	MOVEMENT_KING_CASTLE,
	NUM_MOVEMENT_TYPES
} MovementType;

typedef struct {
	MovementType type;
	int target;
} PossibleMove;

extern const char *MOVEMENT_TYPE_STRINGS[NUM_MOVEMENT_TYPES];

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
