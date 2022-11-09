#include "logic.h"

#include <stdbool.h>
#include <stdio.h>

#include "display.h"
#include "movement.h"

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
		// Move the king and the castle to the other side.
		board[dest] = *selected_piece;
		board[selected] = empty_space;
		// Queen side.
		if (dest - selected < 0) {
			int castle_loc = dest / BOARD_SIZE;
			board[dest + 1] = board[castle_loc];
			board[castle_loc] = empty_space;
			// King size.
		} else {
			int castle_loc = (dest / BOARD_SIZE) + BOARD_SIZE - 1;
			board[dest - 1] = board[castle_loc];
			board[castle_loc] = empty_space;
		}
		break;
	case MOVEMENT_PAWN_EN_PASSANT:
		// Remove target pawn, rest is the same.
		board[dest +
		      (colour == COLOUR_WHITE ? BOARD_SIZE : -BOARD_SIZE)] =
			empty_space;
	case MOVEMENT_PIECE_TAKE:
	case MOVEMENT_PAWN_PROMOTION:
	case MOVEMENT_PAWN_LONG_JUMP:
	case MOVEMENT_NORMAL:
		board[dest] = *selected_piece;
		board[selected] = empty_space;
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
	int king = -1;
	int checks = 0;
	for (int i = 0; i < BOARD_SIZE * BOARD_SIZE; i++) {
		PlayPiece *piece = &board[i];
		if (piece->type == PIECE_KING && piece->colour == player) {
			king = i;
			break;
		}
	}

	// The game hasn't ended but this player does not have a king?!
	if (king == -1) {
		return checks;
	}

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
			if (possible_moves[m].target == king) {
				// printf("Piece %c%c has check on king %c%c\n",
				// INT_TO_COORD(i), INT_TO_COORD(king));
				checks++;
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
					printf("Piece can break check %s (%s) "
					       "%c%c to %c%c\n",
					       CHESS_PIECE_STRINGS[board[i].type],
					       PLAYER_COLOUR_STRINGS[player], INT_TO_COORD(
						       i),
					       INT_TO_COORD(m));
					// view_board(next_board);
					breakable++;
				}
			}
		}
	}
	return breakable;
}
