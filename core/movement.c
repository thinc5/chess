#include "movement_stats.h"
#include "movement.h"
#include "log.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

const EMovementType (*PIECE_MOVEMENT_ALGORITHM[PIECE_NUM_PIECES])(
	Board board, int start, int target, size_t move_count, size_t check) = {
	[PIECE_NONE] = &none_movement_algorithm,
	[PIECE_PAWN] = &pawn_movement_algorithm,
	[PIECE_KNIGHT] = &knight_movement_algorithm,
	[PIECE_ROOK] = &rook_movement_algorithm,
	[PIECE_BISHOP] = &bishop_movement_algorithm,
	[PIECE_QUEEN] = &queen_movement_algorithm,
	[PIECE_KING] = &king_movement_algorithm,
};

static const int POSSIBLE_MOVES[PIECE_NUM_PIECES + 1][11][2] = {
	[PIECE_NONE] = { { 0 } },
	// Include both pawn's possible moves.
	[PIECE_PAWN] =
	{ { -1, 1 }, { 0, 1 }, { 1, 1 }, { 0, 2 }, { -1, -1 }, { 0, -1 },
	  { 1, -1 }, { 0, -2 } },
	[PIECE_KNIGHT] = { { 1, -2 },
			   { 2, -1 },
			   { 2, 1 },
			   { 1, 2 },
			   { -1, 2 },
			   { -2, 1 },
			   { -1, -2 },
			   { -2, -1 } },
	[PIECE_ROOK] = { { 0 } },
	[PIECE_BISHOP] = { { 0 } },
	[PIECE_QUEEN] = { { 0 } },
	[PIECE_KING] = { { -1, 1 },
			 { 0, 1 },
			 { 1, 1 },
			 { 1, 0 },
			 { 2, 0 },
			 { 1, -1 },
			 { 0, -1 },
			 { -1, -1 },
			 { -1, 0 },
			 { -2, 0 } },
	[PIECE_NUM_PIECES] = { { 0 } }
};

static const int POSSIBLE_MOVE_NUMS[PIECE_NUM_PIECES] = {
	[PIECE_NONE] = 0, [PIECE_PAWN] = 8, [PIECE_KNIGHT] = 8,
	[PIECE_ROOK] = 0,
	[PIECE_BISHOP] = 0, [PIECE_QUEEN] = 0, [PIECE_KING] = 10
};

// Useful checks.
static inline bool moving_forward(EPlayerColour colour, int start_y,
				  int target_y)
{
	return(colour ? start_y > target_y : start_y < target_y);
}

static inline bool end_row(EPlayerColour colour, int y)
{
	return y == (colour == COLOUR_WHITE ? BOARD_SIZE - 1 : 0);
}

static inline bool can_move(Board board, int piece, int target)
{
	return piece >= 0 && piece < BOARD_SIZE * BOARD_SIZE && target >= 0 &&
	       target < BOARD_SIZE * BOARD_SIZE &&
	       (board[target].type == PIECE_NONE ||
		board[piece].colour != board[target].colour);
}


static bool parallel_movement(Board board, MoveStats *stats)
{
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

static bool diagonal_movement(Board board, MoveStats *stats)
{
	// No collisions until destination.
	switch (stats->direction) {
	case DIRECTION_NORTH_EAST:
		for (int d = 1; d < stats->dist_x; d++)
			if (board[((stats->start_y + d) * BOARD_SIZE) +
				  stats->start_x + d].type)
				return false;
		break;
	case DIRECTION_SOUTH_EAST:
		for (int d = 1; d < stats->dist_x; d++)
			if (board[((stats->start_y - d) * BOARD_SIZE) +
				  stats->start_x + d].type)
				return false;
		break;
	case DIRECTION_SOUTH_WEST:
		for (int d = 1; d < stats->dist_x; d++)
			if (board[((stats->start_y - d) * BOARD_SIZE) +
				  stats->start_x - d].type)
				return false;
		break;
	case DIRECTION_NORTH_WEST:
		for (int d = 1; d < stats->dist_x; d++)
			if (board[((stats->start_y + d) * BOARD_SIZE) +
				  stats->start_x - d].type)
				return false;
		break;
	default:
		return false;
	}
	return true;
}

// Movement algorithms.
EMovementType none_movement_algorithm(Board board, int start, int target,
				      size_t move_count, size_t check)
{
	return MOVEMENT_ILLEGAL;
}

EMovementType pawn_movement_algorithm(Board board, int start, int target,
				      size_t move_count, size_t check)
{
	if (!can_move(board, start, target)) {
		return MOVEMENT_ILLEGAL;
	}
	MoveStats stats = get_move_stats(start, target);
	// Can only move forward.
	if (!moving_forward(board[start].colour, stats.start_y,
			    stats.target_y)) {
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
			return MOVEMENT_PIECE_CAPTURE;
		}
		// En passant opportunity?
		int dir = (stats.direction == DIRECTION_EAST ||
			   stats.direction == DIRECTION_NORTH_EAST ||
			   stats.direction == DIRECTION_NORTH_EAST)
		  ? 1
		  : -1;
		// Target space has to be empty, enemy pawn the same row.
		if (board[target].type == PIECE_NONE &&
		    board[start].colour != board[start + dir].colour &&
		    board[start + dir].type == PIECE_PAWN &&
		    board[start + dir].last_move == move_count - 1) {
			return MOVEMENT_PAWN_EN_PASSANT;
		}
		return MOVEMENT_ILLEGAL;
	default:
		return MOVEMENT_ILLEGAL;
	}
}

EMovementType knight_movement_algorithm(Board board, int start, int target,
					size_t move_count, size_t check)
{
	if (!can_move(board, start, target))
		return MOVEMENT_ILLEGAL;
	MoveStats stats = get_move_stats(start, target);
	// Two away on x or y always.
	if (!((stats.dist_x == 2 && stats.dist_y == 1) ||
	      (stats.dist_x == 1 && stats.dist_y == 2)))
		return MOVEMENT_ILLEGAL;
	if (board[target].type != PIECE_NONE)
		return MOVEMENT_PIECE_CAPTURE;
	return MOVEMENT_NORMAL;
}

EMovementType rook_movement_algorithm(Board board, int start, int target,
				      size_t move_count, size_t check)
{
	if (!can_move(board, start, target))
		return MOVEMENT_ILLEGAL;
	MoveStats stats = get_move_stats(start, target);
	// No collisions until destination.
	if (!parallel_movement(board, &stats))
		return MOVEMENT_ILLEGAL;
	if (board[target].type != PIECE_NONE)
		return MOVEMENT_PIECE_CAPTURE;

	return MOVEMENT_NORMAL;
}

EMovementType bishop_movement_algorithm(Board board, int start, int target,
					size_t move_count, size_t check)
{
	if (!can_move(board, start, target))
		return MOVEMENT_ILLEGAL;
	MoveStats stats = get_move_stats(start, target);
	// x distance is always equal to the y dist... right angle
	if (stats.dist_x != stats.dist_y)
		return MOVEMENT_ILLEGAL;
	if (!diagonal_movement(board, &stats))
		return MOVEMENT_ILLEGAL;
	if (board[target].type != PIECE_NONE)
		return MOVEMENT_PIECE_CAPTURE;
	return MOVEMENT_NORMAL;
}

EMovementType queen_movement_algorithm(Board board, int start, int target,
				       size_t move_count, size_t check)
{
	if (!can_move(board, start, target))
		return MOVEMENT_ILLEGAL;
	MoveStats stats = get_move_stats(start, target);
	if (!parallel_movement(board,
			       &stats) && !diagonal_movement(board, &stats))
		return MOVEMENT_ILLEGAL;
	if (board[target].type != PIECE_NONE)
		return MOVEMENT_PIECE_CAPTURE;
	return MOVEMENT_NORMAL;
}

EMovementType king_movement_algorithm(Board board, int start, int target,
				      size_t move_count, size_t check)
{
	MoveStats stats = get_move_stats(start, target);
	PlayPiece *king = &board[start];

	// Attempt for castle? Must be king's first move and cannot be in check.
	if (!check && stats.dist_x == 2 && king->moves == 0) {
		int modifier = 0;
		switch (stats.direction) {
		case DIRECTION_EAST: {
			modifier = 1;
		}
		break;
		case DIRECTION_WEST: {
			modifier = -2;
		}
		break;
		default:
			return MOVEMENT_ILLEGAL;
		}

		PlayPiece *rook = &board[target + modifier];

		if (rook->colour !=
		    king->colour ||
		    rook->type != PIECE_ROOK ||
		    rook->moves != 0) {
			return MOVEMENT_ILLEGAL;
		}

		// Check there are no pieces in the way.
		MoveStats rook_stats = get_move_stats(target + modifier,
						      start +
						      (stats.direction ==
						       DIRECTION_EAST ? 1 : -1));
		if (!parallel_movement(board,
				       &rook_stats)) {
			return MOVEMENT_ILLEGAL;
		}
		return MOVEMENT_KING_CASTLE;
	}

	if (!can_move(board, start, target))
		return MOVEMENT_ILLEGAL;

	if (stats.dist_x > 1 || stats.dist_y > 1)
		return MOVEMENT_ILLEGAL;

	if (!parallel_movement(board,
			       &stats) && !diagonal_movement(board, &stats))
		return MOVEMENT_ILLEGAL;

	if (board[target].type != PIECE_NONE)
		return MOVEMENT_PIECE_CAPTURE;

	return MOVEMENT_NORMAL;
}

static void traverse_board(Board board, int location, size_t *possible,
			   PossibleMove possible_moves[MAX_POSSIBLE_MOVES],
			   size_t move_count, size_t check, size_t num_steps,
			   int x_step,
			   int y_step)
{
	// We always start one 'step' from the piece origin.
	// We can only travel as far as the board size in any direction.
	for (int step = 1; step < num_steps; step++) {
		// Apply step to the piece position.
		int original_x = (location % BOARD_SIZE);
		int original_y = (location / BOARD_SIZE);
		int new_x = original_x + (x_step * step);
		int new_y = original_y + (y_step * step);

		// If we have gone out of bounds, we stop.
		if (new_x < 0 || new_x >= BOARD_SIZE)
			break;
		if (new_y < 0 || new_y >= BOARD_SIZE)
			break;

		int step_loc = (new_y * BOARD_SIZE) + new_x;
		EMovementType type =
			PIECE_MOVEMENT_ALGORITHM[board[location].type](
				board, location, step_loc, move_count, check);

		// Would we be moving into one of our own pieces?
		if (type == MOVEMENT_ILLEGAL)
			break;

		// Record valid move.
		possible_moves[(*possible)] = (PossibleMove){
			.type = type,
			.target = step_loc,
		};
		(*possible)++;

		// If it is a capture we stop "stepping".
		if (type == MOVEMENT_PIECE_CAPTURE)
			break;
	}
}

static void
possible_parallel_movements(Board board, int location, size_t *possible,
			    PossibleMove possible_moves[MAX_POSSIBLE_MOVES],
			    size_t move_count, size_t check, size_t num_steps)
{
	// North
	traverse_board(board, location, possible,
		       possible_moves,
		       move_count, check, num_steps, 0, 1);
	// South
	traverse_board(board, location, possible,
		       possible_moves,
		       move_count, check, num_steps, 0, -1);
	// East
	traverse_board(board, location, possible,
		       possible_moves,
		       move_count, check, num_steps, 1, 0);
	// West
	traverse_board(board, location, possible,
		       possible_moves,
		       move_count, check, num_steps, -1, 0);
}

static void
possible_diagonal_movements(Board board, int location, size_t *possible,
			    PossibleMove possible_moves[MAX_POSSIBLE_MOVES],
			    size_t move_count, size_t check, size_t num_steps)
{
	// North East
	traverse_board(board, location, possible,
		       possible_moves,
		       move_count, check, num_steps, -1, 1);
	// South East
	traverse_board(board, location, possible,
		       possible_moves,
		       move_count, check, num_steps, -1, -1);
	// South West
	traverse_board(board, location, possible,
		       possible_moves,
		       move_count, check, num_steps, 1, -1);
	// North West
	traverse_board(board, location, possible,
		       possible_moves,
		       move_count, check, num_steps, 1, 1);
}

static void possible_movements(Board board, int location, size_t *possible,
			       PossibleMove possible_moves[MAX_POSSIBLE_MOVES],
			       size_t move_count, size_t check)
{
	for (int i = 0; i < POSSIBLE_MOVE_NUMS[board[location].type];
	     i++) {
		// Prevent wrap around/overlap.
		int new_x =
			(location % BOARD_SIZE) +
			POSSIBLE_MOVES[board[location].type][i][0];
		int new_y =
			(location / BOARD_SIZE) +
			POSSIBLE_MOVES[board[location].type][i][1];
		if (new_y < 0 || new_y >= BOARD_SIZE ||
		    new_x >= BOARD_SIZE ||
		    new_x < 0) {
			continue;
		}
		int new_loc = location +
			      (POSSIBLE_MOVES[board[location].type][i][0
			       ]) +
			      (POSSIBLE_MOVES[board[location].type][i][1
			       ] * BOARD_SIZE);
		EMovementType type =
			PIECE_MOVEMENT_ALGORITHM[board[location].type](
				board, location, new_loc, move_count,
				check);
		if (type > MOVEMENT_ILLEGAL) {
			possible_moves[*possible] = (PossibleMove){
				.type = type,
				.target = new_loc,
			};
			(*possible)++;
		}
	}
}

size_t
get_possible_moves_for_piece(Board board, int location,
			     PossibleMove possible_moves[MAX_POSSIBLE_MOVES],
			     size_t move_count, size_t check)
{
	size_t possible = 0;
	switch (board[location].type) {
	// Our 'special cases'.
	case PIECE_PAWN:
	case PIECE_KNIGHT:
	case PIECE_KING:
		possible_movements(board, location,
				   &possible,
				   possible_moves,
				   move_count, check);
		break;
	case PIECE_ROOK:
		possible_parallel_movements(board, location,
					    &possible,
					    possible_moves,
					    move_count, check,
					    BOARD_SIZE);
		break;
	case PIECE_BISHOP:
		possible_diagonal_movements(board, location,
					    &possible,
					    possible_moves,
					    move_count, check,
					    BOARD_SIZE);
		break;
	case PIECE_QUEEN:
		possible_parallel_movements(board, location,
					    &possible,
					    possible_moves,
					    move_count, check,
					    BOARD_SIZE);
		possible_diagonal_movements(board, location,
					    &possible,
					    possible_moves,
					    move_count, check,
					    BOARD_SIZE);
		break;
	case PIECE_NONE:
	default:
		break;
	}
	return possible;
}

