#include "pieces.h"
#include "display.h"
#include "replay.h"
#include "san.h"
#include "pgn.h"
#include "log.h"

#include <stdio.h>
#include <ctype.h>
#include <string.h>

static inline bool valid_char_pairing(char x, char y)
{
	return !(tolower(x) < 'a' || tolower(x) > 'a' + BOARD_SIZE || y < '0' ||
		 y > '0' + BOARD_SIZE);
}

static void set_msg(SanData* san_data, char *input_buffer,
		    size_t *input_pointer)
{
	// Convert coordinates to our format and write to input buffer.
	snprintf(input_buffer, 6, "%c%c %c%c",
		 san_data->origin[0] + 'a', san_data->origin[1] + '1',
		 san_data->destination[0] + 'a',
		 san_data->destination[1] + '1');
	*input_pointer = 5;
}

// Determine the origin if missing of a sans formatted.
void determine_origin(SanData *san_data, ChessGame *game)
{
	debug_show_piece((PlayPiece) { .colour = san_data->colour,
				       .type = san_data->piece });
	if (san_data->origin[0] != -1 && san_data->origin[1] != -1)
		return;

	int destination_square = san_data->destination[0] +
				 (san_data->destination[1] *
				  BOARD_SIZE);

	// Now we need to use the board to determine the full coordinates.
	for (int i = 0; i < BOARD_SIZE * BOARD_SIZE; i++) {
		// We have found our start location, we are done here.
		if ((san_data->origin[0] != -1 &&
		     san_data->origin[1] != -1)) {
			break;
		}

		PlayPiece selected = game->board[i];
		if (selected.colour != san_data->colour ||
		    selected.type != san_data->piece) {
			continue;
		}

		game->selected_piece = i;

		// Determine if it can make the proposed move.
		game->num_possible_moves = get_possible_moves_for_piece(
			game->board, game->selected_piece,
			game->possible_moves,
			game->move_count, game->check);

		for (size_t j = 0; j < game->num_possible_moves; j++) {
			if (game->possible_moves[j].target !=
			    destination_square) {
				continue;
			}
			san_data->origin[0] = i % BOARD_SIZE;
			san_data->origin[1] = i / BOARD_SIZE;
			break;
		}
	}

	if (san_data->origin[0] == -1 || san_data->origin[1] == -1) {
		ERROR_LOG("\n!!!!\n!!!!!Unable to determine origin!!!!!\n!!!!\n");
		ERROR_LOG("%c%c\n", san_data->destination[0] + 'a',
			  san_data->destination[1] + '1');
		view_board(game->board, game->selected_piece,
			   game->num_possible_moves, game->possible_moves);
	}
}

EReplayInput read_pgn_move(FILE *file, ChessGame *game,
			   char *input_buffer,
			   size_t *input_pointer)
{
	static int move_num;
	static SanData cached_move;
	// Are we dealing with a cached move?
	if (move_num != 0) {
		move_num = 0;
		determine_origin(&cached_move, game);
		if (cached_move.origin[0] == -1 ||
		    cached_move.destination[0] == -1)
			return REPLAY_INPUT_INVALID;

		set_msg(&cached_move, input_buffer, input_pointer);
		return REPLAY_INPUT_COORDS;
	}

// Reasonable size, we don't expect more than 10000 moves.
	char move_number[5];
	memset(move_number, '\0', 5);
	int move_number_len = 0;
	char current;
// printf("move_umber\n");
	while (1) {
		current = fgetc(file);
		// Game ongoing but replay is finished.
		if (current == '*') {
			DEBUG_LOG("REPLAY OVER, GAME ONGOING\n");
			return REPLAY_INPUT_REPLAY_OVER;
		}
		if (move_number_len == 1) {
			// Game is over and we have a result.
			if (current == '-') {
				EPlayerColour won = move_number[0] ==
						    '0' ? COLOUR_WHITE :
						    COLOUR_BLACK;
				// DEBUG_LOG("REPLAY: GAME OVER, WINNER: %s\n",
				// 	 PLAYER_COLOUR_STRINGS[won]);
				return won ==
				       COLOUR_WHITE ? REPLAY_INPUT_WIN_WHITE :
				       REPLAY_INPUT_WIN_BLACK;
			} else if (current == '/') {
				// DEBUG_LOG("REPLAY: GAME OVER, DRAW\n");
				return REPLAY_INPUT_DRAW;
			}
		}
		if (current >= '0' && current <= '9') {
			if (move_number_len >= 4) {
				ERROR_LOG("Invalid number of moves.\n");
				return REPLAY_INPUT_INVALID;
			}
			move_number[move_number_len] = current;
			move_number_len++;
		} else if (current == '.') {
			break;
		}
	}

	// Set move num.
	move_num = atoi(move_number);
	SanData san_data[2] = { EMPTY_SAN_DATA, EMPTY_SAN_DATA };

	// Parse san moves.
	read_san_moves(file, (SanData *)&san_data);
	cached_move = san_data[1];

	determine_origin(&san_data[0], game);
	if (san_data[0].origin[0] == -1 || san_data[0].destination[0] == -1)
		return REPLAY_INPUT_INVALID;
	set_msg(&san_data[0], input_buffer, input_pointer);
	return REPLAY_INPUT_COORDS;
}

EReplayInput parse_pgn_input(FILE *file, ChessGame *game, char *input_buffer,
			     size_t *input_pointer)
{
	// Read the first character to determine if we are dealing with a comment
	// or some metadata.
	char first = fgetc(file);
	// Rewind.
	fseek(file, -1, SEEK_CUR);
	if (first == EOF) {
		return REPLAY_INPUT_EOF;
	}

	if (first == '[' || first == '{' || first == ';') {
		read_line_from_file(file, input_buffer, input_pointer);
		return REPLAY_INPUT_COMMENT;
	}

	return read_pgn_move(file, game, input_buffer, input_pointer);
}

EReplayInput read_next_move_pgn(FILE *file, ChessGame *game,
				char *input_buffer,
				size_t *input_pointer)
{
	return parse_pgn_input(file, game,
			       input_buffer,
			       input_pointer);
}
