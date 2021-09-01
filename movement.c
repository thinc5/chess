#include <stdbool.h>
#include <stdlib.h>

#include <stdio.h>

#include "movement.h"

const char *MOVEMENT_TYPE_STRINGS[NUM_MOVEMENT_TYPES] = {
    "MOVEMENT_ILLEGAL",
    "MOVEMENT_NORMAL",
    "MOVEMENT_PIECE_TAKE",
    "MOVEMENT_PIECE_TAKE",
    "PAWN_UPGRADE_MOVE",
};

const MovementType (*PIECE_MOVEMENT_ALGORITHM[NUM_CHESS_PIECES])(Board board, int start, int target, size_t move_count) = {
    [PIECE_NONE] = &none_movement_algorithm,
    [PIECE_PAWN] = &pawn_movement_algorithm,
    [PIECE_KNIGHT] = &knight_movement_algorithm,
    [PIECE_ROOK] = &rook_movement_algorithm,
    [PIECE_BISHOP] = &bishop_movement_algorithm,
    [PIECE_QUEEN] = &queen_movement_algorithm,
    [PIECE_KING] = &king_movement_algorithm,
};

static const int POSSIBLE_MOVES[NUM_CHESS_PIECES][8][2] = {
    [PIECE_NONE] = {{0}},
    // Include both pawn's possible moves.
    [PIECE_PAWN] = {{-1, 1}, {0, 1}, {1, 1}, {0, 2}, {-1, -1}, {0, -1}, {1, -1}, {0, -2}},
    [PIECE_KNIGHT] = {{1, -2}, {2, -1}, {2, 1}, {1, 2}, {-1, 2}, {-2, 1}, {-1, -2}, {-2, -1}},
    [PIECE_ROOK] = {{0}},
    [PIECE_BISHOP] = {{0}},
    [PIECE_QUEEN] = {{0}},
    [PIECE_KING] = {{-1, 1}, {0, 1}, {1, 1}, {1, 0}, {1, -1}, {0, -1}, {-1, -1}, {-1, 0}}
};

static const int POSSIBLE_MOVE_NUMS[NUM_CHESS_PIECES] = {
    [PIECE_NONE] = 0,
    [PIECE_PAWN] = 8,
    [PIECE_KNIGHT] = 8,
    [PIECE_ROOK] = 0,
    [PIECE_BISHOP] = 0,
    [PIECE_QUEEN] = 0,
    [PIECE_KING] = 8
};

typedef enum {
    DIRECTION_NONE,
    DIRECTION_NORTH,
    DIRECTION_EAST,
    DIRECTION_SOUTH,
    DIRECTION_WEST,
    DIRECTION_NORTH_EAST,
    DIRECTION_SOUTH_EAST,
    DIRECTION_SOUTH_WEST,
    DIRECTION_NORTH_WEST,
    NUM_DIRECTIONS
} MovementDirection;

const char *MOVEMENT_DIRECTION_STRINGS[NUM_DIRECTIONS] = {
    [DIRECTION_NONE]        = "DIRECTION_NONE",
    [DIRECTION_NORTH]       = "DIRECTION_NORTH",
    [DIRECTION_EAST]        = "DIRECTION_EAST",
    [DIRECTION_SOUTH]       = "DIRECTION_SOUTH",
    [DIRECTION_WEST]        = "DIRECTION_WEST",
    [DIRECTION_NORTH_EAST]  = "DIRECTION_NORTH_EAST",
    [DIRECTION_SOUTH_EAST]  = "DIRECTION_SOUTH_EAST",
    [DIRECTION_SOUTH_WEST]  = "DIRECTION_SOUTH_WEST",
    [DIRECTION_NORTH_WEST]  = "DIRECTION_NORTH_WEST",
};

// static const int MOVE_DIRECTION_VALUES[NUM_DIRECTIONS] = {
//     [DIRECTION_NONE]        = 0x00,
//     [DIRECTION_NORTH]       = 0x01,
//     [DIRECTION_EAST]        = 0x02,
//     [DIRECTION_SOUTH]       = 0x04,
//     [DIRECTION_WEST]        = 0x08,
//     [DIRECTION_NORTH_EAST]  = 0x01 | 0x02,
//     [DIRECTION_SOUTH_EAST]  = 0x04 | 0x02,
//     [DIRECTION_SOUTH_WEST]  = 0x04 | 0x08,
//     [DIRECTION_NORTH_WEST]  = 0x01 | 0x08
// };

typedef struct {
    int start_x;
    int target_x;
    int dist_x;
    int start_y;
    int target_y;
    int dist_y;
    MovementDirection direction;
} MoveStats;

static inline MovementDirection get_movement_direction(int start_x, int start_y, int target_x, int target_y) {
    MovementDirection x_dir = start_x == target_x ? DIRECTION_NONE : (start_x < target_x ? DIRECTION_EAST : DIRECTION_WEST);
    MovementDirection y_dir = start_y == target_y ? DIRECTION_NONE : (start_y < target_y ? DIRECTION_NORTH : DIRECTION_SOUTH);
    if (x_dir && y_dir) {
        if (x_dir == DIRECTION_EAST) {
            if (y_dir == DIRECTION_NORTH) {
                return DIRECTION_NORTH_EAST;
            } else if (y_dir == DIRECTION_SOUTH) {
                return DIRECTION_SOUTH_EAST;
            }
        } else if (x_dir == DIRECTION_WEST) {
            if (y_dir == DIRECTION_NORTH) {
                return DIRECTION_NORTH_WEST;
            } else if (y_dir == DIRECTION_SOUTH) {
                return DIRECTION_SOUTH_WEST;
            }
        }
    }
    return x_dir || y_dir;
}

static inline MoveStats get_move_stats(int start, int target) {
    int start_x = start % BOARD_SIZE;
    int start_y = start / BOARD_SIZE;
    int target_x = target % BOARD_SIZE;
    int target_y = target / BOARD_SIZE;
    return (MoveStats) {
        .start_x = start_x,
        .target_x = target_x,
        .dist_x = abs(target_x - start_x),
        .start_y = start_y,
        .target_y = target_y,
        .dist_y = abs(target_y - start_y),
        .direction = get_movement_direction(start_x, start_y, target_x, target_y),
    };
}

static inline bool moving_forward(PlayerColour colour, int start_y, int target_y) {
    return (colour ? start_y > target_y : start_y < target_y);
}

static inline bool end_row(PlayerColour colour, int y) {
    return y == (colour == COLOUR_WHITE ? BOARD_SIZE - 1 : 0);
}

static inline bool parallel_movement(Board board, MoveStats *stats) {
    // printf("Direction: %s\n", MOVEMENT_DIRECTION_STRINGS[stats->direction]);
    switch (stats->direction) {
        case DIRECTION_NORTH:
            for (int y = stats->start_y + 1; y < stats->target_y; y++) {
                if (board[(y * BOARD_SIZE) + stats->start_x].type) {
                    return false;
                }
            }
            break;
        case DIRECTION_SOUTH:
            for (int y = stats->target_y + 1; y < stats->start_y; y++) {
                if (board[(y * BOARD_SIZE) + stats->start_x].type)
                    return false;
            }
            break;
        case DIRECTION_EAST:
            for (int x = stats->start_x + 1; x < stats->target_x; x++) {
                if (board[(stats->start_y * BOARD_SIZE) + x].type)
                    return false;
            }
            break;
        case DIRECTION_WEST:
            for (int x = stats->target_x + 1; x < stats->start_x; x++) {
                if (board[(stats->start_y * BOARD_SIZE) + x].type)
                    return false;
            }
            break;
        default:
            return false;
    }
    return true;
}

static inline bool diagonal_movement(Board board, MoveStats *stats) {
    // No collisions until destination.
    switch (stats->direction) {
        case DIRECTION_NORTH_EAST:
            for (int d = 1; d < stats->dist_x; d++)
                if (board[((stats->start_y + d) * BOARD_SIZE) + stats->start_x + d].type)
                    return false;
            break;
        case DIRECTION_SOUTH_EAST:
            for (int d = 1; d < stats->dist_x; d++)
                if (board[((stats->start_y - d) * BOARD_SIZE) + stats->start_x + d].type)
                    return false;
            break;
        case DIRECTION_SOUTH_WEST:
            for (int d = 1; d < stats->dist_x; d++)
                if (board[((stats->start_y - d) * BOARD_SIZE) + stats->start_x - d].type)
                    return false;
            break;
        case DIRECTION_NORTH_WEST:
            for (int d = 1; d < stats->dist_x; d++)
                if (board[((stats->start_y + d) * BOARD_SIZE) + stats->start_x - d].type)
                    return false;
            break;
        default:
            return false;
    }
    return true;
}

static inline bool can_move(Board board, int piece, int target) {
    return piece >= 0 && piece < BOARD_SIZE * BOARD_SIZE && target >= 0 && target < BOARD_SIZE * BOARD_SIZE &&
            (board[target].type == PIECE_NONE || board[piece].colour != board[target].colour);
}

MovementType none_movement_algorithm(Board board, int start, int target, size_t move_count) {
    return MOVEMENT_ILLEGAL;
}

MovementType pawn_movement_algorithm(Board board, int start, int target, size_t move_count) {
    if (!can_move(board, start, target)) {
        return MOVEMENT_ILLEGAL;
    }
    MoveStats stats = get_move_stats(start, target);
    // Can only move forward.
    if (!moving_forward(board[start].colour, stats.start_y, stats.target_y)) {
        return MOVEMENT_ILLEGAL;
    }
    switch (stats.dist_y) {
        // Attempting pawn "long jump".
        case 2:
            // First move?
            if (board[start].moves != 0) {
                return MOVEMENT_ILLEGAL;
            }
            // No horizontal movement?
            if (stats.dist_x != 0) {
                return MOVEMENT_ILLEGAL;
            }
            // No piece in the way?
            if (board[target].type != PIECE_NONE) {
                return MOVEMENT_ILLEGAL;
            }
            // Can move!
            return MOVEMENT_PAWN_LONG_JUMP;
        // Pawn normal move/take piece.
        case 1:
            // Moving forward straight one.
            if (stats.dist_x == 0) {
                // Check that we have no piece in the dest.
                if (board[target].type != PIECE_NONE) {
                    return MOVEMENT_ILLEGAL;
                }
                if (end_row(board[start].colour, stats.target_y)) {
                    return MOVEMENT_PAWN_PROMOTION;
                }
                return MOVEMENT_NORMAL;
            }
            // Diagonal move (taking a piece).
            if (stats.dist_x != 1) {
                return MOVEMENT_ILLEGAL;
            }
            // Piece at target that is not our own?
            if (board[target].type != PIECE_NONE &&
                    board[start].colour != board[target].colour) {
                return MOVEMENT_PIECE_TAKE;
            }
            // En passant opportunity?
            int dir = (stats.direction == DIRECTION_EAST || stats.direction == DIRECTION_NORTH_EAST || stats.direction == DIRECTION_NORTH_EAST) ? 1 : -1;
            // Target space has to be empty, enemy pawn the same row.
            if (board[target].type == PIECE_NONE && board[start].colour != board[start + dir].colour &&
                    board[start + dir].type == PIECE_PAWN && board[start + dir].last_move == move_count - 1) {
                return MOVEMENT_PAWN_EN_PASSANT;
            }
            return MOVEMENT_ILLEGAL;
        default:
            return MOVEMENT_ILLEGAL;
    }
}

MovementType knight_movement_algorithm(Board board, int start, int target, size_t move_count) {
    if (!can_move(board, start, target))
        return MOVEMENT_ILLEGAL;
    MoveStats stats = get_move_stats(start, target);
    // Two away on x or y always.
    if (!((stats.dist_x == 2 && stats.dist_y == 1) ||
            (stats.dist_x == 1 && stats.dist_y == 2)))
        return MOVEMENT_ILLEGAL;
    if (board[target].type != PIECE_NONE)
        return MOVEMENT_PIECE_TAKE;
    return MOVEMENT_NORMAL;
}

MovementType rook_movement_algorithm(Board board, int start, int target, size_t move_count) {
    if (!can_move(board, start, target))
        return MOVEMENT_ILLEGAL;
    MoveStats stats = get_move_stats(start, target);
    // No collisions until destination.
    if (!parallel_movement(board, &stats))
        return MOVEMENT_ILLEGAL;
    if (board[target].type != PIECE_NONE)
        return MOVEMENT_PIECE_TAKE;
    return MOVEMENT_NORMAL;
}

MovementType bishop_movement_algorithm(Board board, int start, int target, size_t move_count) {
    if (!can_move(board, start, target))
        return MOVEMENT_ILLEGAL;
    MoveStats stats = get_move_stats(start, target);
    // x distance is always equal to the y dist... right angle
    if (stats.dist_x != stats.dist_y)
        return MOVEMENT_ILLEGAL;
    if (!diagonal_movement(board, &stats))
        return MOVEMENT_ILLEGAL;
    if (board[target].type != PIECE_NONE)
        return MOVEMENT_PIECE_TAKE;
    return MOVEMENT_NORMAL;
}

MovementType queen_movement_algorithm(Board board, int start, int target, size_t move_count) {
    if (!can_move(board, start, target))
        return MOVEMENT_ILLEGAL;
    MoveStats stats = get_move_stats(start, target);
    if (!parallel_movement(board, &stats) && !diagonal_movement(board, &stats))
        return MOVEMENT_ILLEGAL;
    if (board[target].type != PIECE_NONE)
        return MOVEMENT_PIECE_TAKE;
    return MOVEMENT_NORMAL;
}

MovementType king_movement_algorithm(Board board, int start, int target, size_t move_count) {
    if (!can_move(board, start, target)) {
        return MOVEMENT_ILLEGAL;
    }
    MoveStats stats = get_move_stats(start, target);
    // Attempt for castle?
    if (stats.dist_x == 2) {

    }
    if (stats.dist_x > 1 || stats.dist_y > 1) {
        return MOVEMENT_ILLEGAL;
    }
    if (!parallel_movement(board, &stats) && !diagonal_movement(board, &stats)) 
        return MOVEMENT_ILLEGAL;
    }
    if (board[target].type != PIECE_NONE) {
        return MOVEMENT_PIECE_TAKE;
    }
    return MOVEMENT_NORMAL;
}

static inline void possible_parallel_movements(Board board, int location, size_t *possible, PossibleMove possible_moves[MAX_POSSIBLE_MOVES], size_t move_count) {
    // North
    for (int y = 1; y <  BOARD_SIZE; y++) {
        // Prevent wrap around/overlap.
        int new_y = (location / BOARD_SIZE) + y;
        if (new_y >= BOARD_SIZE) {
            break;
        }
        int new_loc = location + (y * BOARD_SIZE);
        MovementType type = PIECE_MOVEMENT_ALGORITHM[board[location].type](board, location, new_loc, move_count);
        if (type == MOVEMENT_ILLEGAL) {
            break;
        }
		possible_moves[(*possible)] = (PossibleMove) {
            .type = type,
            .target = new_loc,
        };
	    (*possible)++;
        if (type == MOVEMENT_PIECE_TAKE) {
            break;
        }
    }
    // South
    for (int y = -1; y > -BOARD_SIZE; y--) {
        // Prevent wrap around/overlap.
        int new_y = (location / BOARD_SIZE) - y;
        if (new_y < 0) {
            break;
        }
        int new_loc = location + (y * BOARD_SIZE);
        MovementType type = PIECE_MOVEMENT_ALGORITHM[board[location].type](board, location, new_loc, move_count);
        if (type == MOVEMENT_ILLEGAL) {
            break;
        }
		possible_moves[(*possible)] = (PossibleMove) {
            .type = type,
            .target = new_loc,
        };
	    (*possible)++;
        if (type == MOVEMENT_PIECE_TAKE) {
            break;
        }
    }
    // East
    for (int x = 1; x < BOARD_SIZE; x++) {
        // Prevent wrap around/overlap.
        int new_x = (location % BOARD_SIZE) + x;
        if (new_x >= BOARD_SIZE) {
            break;
        }
        int new_loc = location + x;
        MovementType type = PIECE_MOVEMENT_ALGORITHM[board[location].type](board, location, new_loc, move_count);
        if (type == MOVEMENT_ILLEGAL) {
            break;
        }
		possible_moves[(*possible)] = (PossibleMove) {
            .type = type,
            .target = new_loc,
        };
	    (*possible)++;
        if (type == MOVEMENT_PIECE_TAKE) {
            break;
        }
    }
    // West
    for (int x = -1; x > -BOARD_SIZE; x--) {
        // Prevent wrap around/overlap.
        int new_x = (location % BOARD_SIZE) - x;
        if (new_x < 0) {
            break;
        }
        int new_loc = location + x;
        MovementType type = PIECE_MOVEMENT_ALGORITHM[board[location].type](board, location, new_loc, move_count);
        if (type == MOVEMENT_ILLEGAL) {
            break;
        }
		possible_moves[(*possible)] = (PossibleMove) {
            .type = type,
            .target = new_loc,
        };
	    (*possible)++;
        if (type == MOVEMENT_PIECE_TAKE) {
            break;
        }
    }
}

static inline void possible_diagonal_movements(Board board, int location, size_t *possible, PossibleMove possible_moves[MAX_POSSIBLE_MOVES], size_t move_count) {
    // North East
    for (int d = 1; d < BOARD_SIZE; d++) {
        // Prevent wrap around/overlap.
        int new_y = (location / BOARD_SIZE) + d;
        int new_x = (location % BOARD_SIZE) + d;
        if (new_y >= BOARD_SIZE || new_x >= BOARD_SIZE) {
            break;
        }
        int new_loc = location + (d * BOARD_SIZE) + d;
        MovementType type = PIECE_MOVEMENT_ALGORITHM[board[location].type](board, location, new_loc, move_count);
        if (type == MOVEMENT_ILLEGAL) {
            break;
        }
		possible_moves[(*possible)] = (PossibleMove) {
            .type = type,
            .target = new_loc,
        };
	    (*possible)++;
        if (type == MOVEMENT_PIECE_TAKE) {
            break;
        }
    }
    // South East
    for (int d = 1; d < BOARD_SIZE; d++) {
        int new_y = (location / BOARD_SIZE) - d;
        int new_x = (location % BOARD_SIZE) + d;
        if (new_y < 0 || new_x >= BOARD_SIZE) {
            break;
        }
        int new_loc = location + (-d * BOARD_SIZE) + d;
        MovementType type = PIECE_MOVEMENT_ALGORITHM[board[location].type](board, location, new_loc, move_count);
        if (type == MOVEMENT_ILLEGAL) {
            break;
        }
		possible_moves[(*possible)] = (PossibleMove) {
            .type = type,
            .target = new_loc,
        };
	    (*possible)++;
        if (type == MOVEMENT_PIECE_TAKE) {
            break;
        }
    }
    // South West
    for (int d = 1; d < BOARD_SIZE; d++) {
        int new_y = (location / BOARD_SIZE) - d;
        int new_x = (location % BOARD_SIZE) - d;
        if (new_y < 0 || new_x < 0) {
            break;
        }
        int new_loc = location + (-d * BOARD_SIZE) - d;
        MovementType type = PIECE_MOVEMENT_ALGORITHM[board[location].type](board, location, new_loc, move_count);
        if (type == MOVEMENT_ILLEGAL) {
            break;
        }
		possible_moves[(*possible)] = (PossibleMove) {
            .type = type,
            .target = new_loc,
        };
	    (*possible)++;
        if (type == MOVEMENT_PIECE_TAKE) {
            break;
        }
    }
    // North West
    for (int d = 1; d < BOARD_SIZE; d++) {
        int new_y = (location / BOARD_SIZE) + d;
        int new_x = (location % BOARD_SIZE) - d;
        if (new_y >= BOARD_SIZE || new_x < 0) {
            break;
        }
        int new_loc = location + (d * BOARD_SIZE) - d;
        MovementType type = PIECE_MOVEMENT_ALGORITHM[board[location].type](board, location, new_loc, move_count);
        if (type == MOVEMENT_ILLEGAL) {
            break;
        }
		possible_moves[(*possible)] = (PossibleMove) {
            .type = type,
            .target = new_loc,
        };
	    (*possible)++;
        if (type == MOVEMENT_PIECE_TAKE) {
            break;
        }
    }
}

size_t get_possible_moves_for_piece(Board board, int location, PossibleMove possible_moves[MAX_POSSIBLE_MOVES], size_t move_count) {
    size_t possible = 0;
    switch (board[location].type) {
        case PIECE_PAWN:
        case PIECE_KNIGHT:
        case PIECE_KING: {
                for (int i = 0; i < POSSIBLE_MOVE_NUMS[board[location].type]; i++) {
                    int new_loc = location + (POSSIBLE_MOVES[board[location].type][i][0]) + (POSSIBLE_MOVES[board[location].type][i][1] * BOARD_SIZE);
                    MovementType type = PIECE_MOVEMENT_ALGORITHM[board[location].type](board, location, new_loc, move_count);
		            if (type > MOVEMENT_ILLEGAL) {
			            possible_moves[possible] = (PossibleMove) {
                            .type = type,
                            .target = new_loc,
                        };
			            possible++;
                    }
                }
            }
            break;
        case PIECE_ROOK:
            possible_parallel_movements(board, location, &possible, possible_moves, move_count);
            break;
        case PIECE_BISHOP:
            possible_diagonal_movements(board, location, &possible, possible_moves, move_count);
            break;
        case PIECE_QUEEN:
            possible_parallel_movements(board, location, &possible, possible_moves, move_count);
            possible_diagonal_movements(board, location, &possible, possible_moves, move_count);
            break;
        case PIECE_NONE:
        default:
            break;
    }
    return possible;
}
