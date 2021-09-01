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
    NUM_MOVEMENT_TYPES
} MovementType;

typedef struct {
    MovementType type;
    int target;
} PossibleMove;

extern const char *MOVEMENT_TYPE_STRINGS[NUM_MOVEMENT_TYPES];

MovementType none_movement_algorithm(Board board, int start, int target, size_t move_count);
MovementType pawn_movement_algorithm(Board board, int start, int target, size_t move_count);
MovementType knight_movement_algorithm(Board board, int start, int target, size_t move_count);
MovementType rook_movement_algorithm(Board board, int start, int target, size_t move_count);
MovementType bishop_movement_algorithm(Board board, int start, int target, size_t move_count);
MovementType queen_movement_algorithm(Board board, int start, int target, size_t move_count);
MovementType king_movement_algorithm(Board board, int start, int target, size_t move_count);

extern const MovementType (*PIECE_MOVEMENT_ALGORITHM[NUM_CHESS_PIECES])(Board board, int start, int target, size_t move_count);

size_t get_possible_moves_for_piece(Board board, int location, PossibleMove possible_moves[MAX_POSSIBLE_MOVES], size_t move_count);

#endif
