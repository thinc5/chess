#include "logic.h"
#include "display.h"
#include "movement.h"
#include "log.h"

#include <stdbool.h>
#include <stdio.h>
#include <assert.h>

/**
 * Process the piece's movement at a basic level without any input from the
 * user.
 */
MovementType process_movement(Board board, size_t move_count, int selected,
			      int dest, size_t check)
{
	// Check this piece's movement rules.
	PlayPiece *selected_piece = &board[selected];
	PlayerColour colour = selected_piece->colour;
	MovementType movement = PIECE_MOVEMENT_ALGORITHM[selected_piece->type](
		board, selected, dest, move_count, check);
	// printf("Movement result: %s\n", MOVEMENT_TYPE_STRINGS[movement]);

	switch (movement) {
	case MOVEMENT_KING_CASTLE:
		assert(board[dest].type == PIECE_NONE);
		assert(board[selected].type == PIECE_KING);
		// Move the king and the castle to the other side.
		// Save empty space.
		board[dest] = *selected_piece;
		// Save king.
		board[selected] = empty_space;
		int selected_x = selected % BOARD_SIZE;
		int selected_y = selected / BOARD_SIZE;
		// Queen side or King side.
		bool queen_side = BOARD_SIZE - selected_x > BOARD_SIZE / 2;
		int rook_loc = (queen_side ? 0 : BOARD_SIZE - 1) + selected_y *
			       BOARD_SIZE;

		assert(board[rook_loc].type == PIECE_ROOK);
		// Move rook left or right depending on castle side.
		int new_rook_loc = rook_loc == 0 ? dest + 1 : dest - 1;
		board[new_rook_loc] = board[rook_loc];
		// Insert empty space.
		board[rook_loc] = empty_space;
		break;
	case MOVEMENT_PAWN_EN_PASSANT:
		// Remove target pawn.
		board[dest +
		      (colour == COLOUR_WHITE ? BOARD_SIZE : -BOARD_SIZE)] =
			empty_space;
	case MOVEMENT_PIECE_CAPTURE:
	case MOVEMENT_PAWN_PROMOTION:
	case MOVEMENT_PAWN_LONG_JUMP:
	case MOVEMENT_NORMAL:
		DEBUG_LOG("Moving %d\n", (*selected_piece).type);
		board[dest] = *selected_piece;
		board[selected] = empty_space;
		DEBUG_LOG("Moved %d\n", board[dest].type);
		break;
	default:
		break;
	}

	// Move the piece.
	board[dest].moves++;
	board[dest].last_move = move_count++;
	return movement;
}

/**
 * Is this player in checkmate?
 */
size_t is_checkmate_for_player(Board board, PlayerColour player,
			       size_t move_count, size_t check)
{
	// Locate the king of this colour.
	int king_loc = -1;
	int checks = 0;
	for (int i = 0; i < BOARD_SIZE * BOARD_SIZE; i++) {
		PlayPiece *piece = &board[i];
		if (piece->type == PIECE_KING && piece->colour == player) {
			king_loc = i;
			break;
		}
	}

	// The game hasn't ended but this player does not have a king?!
	assert(king_loc != -1);
	assert(board[king_loc].type == PIECE_KING);

	// Do any opposing pieces have a move that could target this king?
	for (int i = 0; i < BOARD_SIZE * BOARD_SIZE; i++) {
		// Skip empty pieces and pieces of this player's colour.
		if (board[i].type == PIECE_NONE || board[i].colour == player) {
			continue;
		}
		// Get this pieces possible moves.
		PossibleMove possible_moves[MAX_POSSIBLE_MOVES] = { 0 };
		size_t num_possible_moves =
			get_possible_moves_for_piece(board, i, possible_moves,
						     move_count, 0);
		if (num_possible_moves < 1) {
			continue;
		}
		// Check if any of the moves target the king.
		for (int m = 0; m < num_possible_moves; m++) {
			if (possible_moves[m].target == king_loc) {
				// printf("Piece %c%c has check on king %c%c\n",
				//        INT_TO_COORD(i), INT_TO_COORD(king_loc));
				checks++;
				// for (int mp = 0; mp < num_possible_moves;
				//      mp++) {
				// 	printf("Piece %c%c moves: %c%c\n",
				// 	       INT_TO_COORD(i),
				// 	       INT_TO_COORD(possible_moves[mp].
				// 			    target));
				// }
				break;
			}
		}
	}
	return checks;
}

/**
 * Is the game in a stalemate? Can this player make even one move?
 */
size_t is_game_stalemate(Board board, PlayerColour player, size_t move_count)
{
	size_t possible = 0;
	for (int i = 0; i < BOARD_SIZE * BOARD_SIZE; i++) {
		if (board[i].colour != player) {
			continue;
		}
		PossibleMove possible_moves[MAX_POSSIBLE_MOVES] = { 0 };
		size_t num_possible_moves =
			get_possible_moves_for_piece(board, i, possible_moves,
						     move_count, 0);
		possible += num_possible_moves;
	}
	return possible;
}

/**
 * Check if this player can make any more moves.
 */
size_t is_game_over_for_player(Board board, Board next_board,
			       PlayerColour player, size_t move_count,
			       size_t check)
{
	size_t breakable = 0;
	int king = -1;
	for (int i = 0; i < BOARD_SIZE * BOARD_SIZE; i++) {
		PlayPiece *piece = &board[i];
		if (piece->type == PIECE_KING && piece->colour == player) {
			king = i;
			break;
		}
	}
	// The game hasn't ended but this player does not have a king?!
	if (king == -1) {
		return breakable;
	}

	for (int i = 0; i < BOARD_SIZE * BOARD_SIZE; i++) {
		// Only look at non empty spaces, and pieces that this player can
		// move.
		if (board[i].type == PIECE_NONE || board[i].colour != player) {
			continue;
		}
		// printf("Checking if piece %c%c breaks checkmate\n",
		// INT_TO_COORD(i)); Get possible moves.
		PossibleMove possible_moves[MAX_POSSIBLE_MOVES] = { 0 };
		size_t possible = get_possible_moves_for_piece(board, i,
							       possible_moves,
							       move_count,
							       check);
		if (possible < 1) {
			continue;
		}
		// Check if the moves cancel the check.
		for (int m = 0; m < possible; m++) {
			// Process all movements for this piece, and if the
			// movement is valid, see if it saves the king.
			if (process_movement(next_board, move_count, i,
					     possible_moves[m].target,
					     check) != MOVEMENT_ILLEGAL) {
				size_t checkmate =
					is_checkmate_for_player(next_board,
								player,
								move_count,
								check);
				// Reset the state.
				set_board(board, next_board);
				if (checkmate < 1) {
					// DEBUG_LOG("Piece can break check %s (%s) "
					//        "%c%c to %c%c\n",
					//        CHESS_PIECE_STRINGS[board[i].type],
					//        PLAYER_COLOUR_STRINGS[player], INT_TO_COORD(
					// 	       i),
					//        INT_TO_COORD(m));
					// view_board(next_board);
					breakable++;
				}
			}
		}
	}
	return breakable;
}
