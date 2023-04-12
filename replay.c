#include "replay.h"
#include "display.h"
#include "game.h"
#include "logic.h"
#include "pgn.h"
#include "log.h"

#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

char *REPLAY_INPUT_STRINGS[REPLAY_INPUT_NUM_TYPES] = {
	"REPLAY_INPUT_INVALID",
	"REPLAY_INPUT_COMMENT",
	"REPLAY_INPUT_COORDS",
	"REPLAY_INPUT_DRAW",
	"REPLAY_INPUT_WIN_WHITE",
	"REPLAY_INPUT_WIN_BLACK",
	"REPLAY_INPUT_REPLAY_OVER",
	"REPLAY_INPUT_EOF"
};

typedef enum {
	FORMAT_INVALID,
	FORMAT_RAW,
	FORMAT_PGN,
	FORMAT_NUM_FILE_FORMATS,
} EFileFormat;

const char *FILE_FORMAT_EXTENSIONS[] = {
	[FORMAT_RAW] = "raw",
	[FORMAT_PGN] = "pgn"
};

EFileFormat determine_file_format(const char *filepath)
{
	// Warning: this function is lazy and assumes that all files have an
	// 'extention', and that they are what they say they are.
	// Find the last occurrence of '.'.
	const char *ext_loc = strchr(filepath, '.');
	const char *next_loc = ext_loc;

	while ((next_loc = strchr(ext_loc, '.')) != NULL) {
		if (next_loc != NULL && strlen(next_loc) > 1) {
			ext_loc = next_loc + 1;
		} else {
			ext_loc = next_loc;
		}
	}


	if (ext_loc == NULL) {
		return FORMAT_INVALID;
	}

	// Compare against suppored file extensions.
	for (EFileFormat format = FORMAT_RAW; format < FORMAT_NUM_FILE_FORMATS;
	     format++) {
		if (strncmp(ext_loc, FILE_FORMAT_EXTENSIONS[format],
			    strlen(FILE_FORMAT_EXTENSIONS[format])) == 0)
			return format;
	}
	return FORMAT_INVALID;
}

int read_char_from_file(FILE *file, char *input_buffer,
			size_t *input_pointer)
{
	char c = fgetc(file);
	if (c == EOF) {
		return 0;
	}
	input_buffer[(*input_pointer)++] = c;
	return 1;
}

int read_line_from_file(FILE *file, char *input_buffer,
			size_t *input_pointer)
{
	char c;
	while (true) {
		c = fgetc(file);
		if (c == EOF) {
			return 0;
		} else if (c == '\n') {
			return *input_pointer;
		} else {
			input_buffer[(*input_pointer)++] = c;
		}
	}
}

EReplayInput read_next_move_raw(FILE *file, ChessGame *game, char *input_buffer,
				size_t *input_pointer)
{
	int result = read_line_from_file(file, input_buffer, input_pointer);
	if (result == 0 && *input_pointer == 0)
		return REPLAY_INPUT_EOF;

	if (input_buffer[0] == '#')
		return REPLAY_INPUT_COMMENT;

	if (*input_pointer == 2 &&
	    !(tolower(input_buffer[0]) < 'a' ||
	      tolower(input_buffer[0]) > 'a' + BOARD_SIZE ||
	      input_buffer[1] < '0' ||
	      input_buffer[1] > '0' + BOARD_SIZE))
		return REPLAY_INPUT_COORDS;

	if (strcmp(input_buffer, "ff") == 0)
		return game->turn ==
		       COLOUR_WHITE ? REPLAY_INPUT_WIN_WHITE :
		       REPLAY_INPUT_WIN_BLACK;

	return REPLAY_INPUT_INVALID;
}

EReplayInput (*READ_NEXT_MOVE[])(FILE *file, ChessGame *game,
				 char *input_buffer,
				 size_t *input_pointer) = {
	[FORMAT_RAW] = &read_next_move_raw,
	[FORMAT_PGN] = &read_next_move_pgn
};

void replay_chess(ChessGame *game, const char *filepath)
{
	INFO_LOG("Attempting to replay: %s\n", filepath);
	// Read the extention of the file and determine what it claims to be.
	EFileFormat format = determine_file_format(filepath);

	// Does file exist?
	FILE *file = fopen(filepath, "r");
	if (file == NULL) {
		ERROR_LOG("File does not exist or cannot be read.");
		return;
	}

	DEBUG_LOG("Processing file of type %s\n",
		  FILE_FORMAT_EXTENSIONS[format]);

	while (true) {
		// Is the game over?
		game->check = is_checkmate_for_player(game->board,
						      game->turn,
						      game->move_count,
						      game->check);
		bool game_over = game->check && !is_game_over_for_player(
			game->board,
			game->next_board,
			game->turn,
			game->move_count,
			game->check);
		if (game_over) {
			printf("Checkmate! player %d (%s) wins "
			       "with %ld moves!\n",
			       ((game->turn + 1) %
				PLAYER_NUM_COLOURS) + 1,
			       PLAYER_COLOUR_STRINGS[(game->turn
						      + 1) %
						     PLAYER_NUM_COLOURS],
			       game->move_count);
			break;
		} else if (game->check) {
			printf(
				"%s king in check!\n",
				PLAYER_COLOUR_STRINGS[game->turn]);
		}

		// Reset move buffer.
		memset(game->input_buffer, 0, INPUT_BUFFER_SIZE);
		game->input_pointer = 0;

		EReplayInput move_result =
			READ_NEXT_MOVE[format](file, game,
					       game->input_buffer,
					       &game->input_pointer);

		// DEBUG_LOG("%s\n", game->input_buffer);

		// We handle special replay specific cases here.
		DEBUG_LOG("MOVE RESULT: %s\n",
			  REPLAY_INPUT_STRINGS[move_result]);
		switch (move_result) {
		case REPLAY_INPUT_COMMENT:
			INFO_LOG("(COMMENT) %s\n", game->input_buffer);
			continue;
		case REPLAY_INPUT_REPLAY_OVER:
			INFO_LOG("Replay over but game ongoing\n");
			fclose(file);
			return;
		case REPLAY_INPUT_DRAW:
			INFO_LOG("Game ended in draw\n");
			fclose(file);
			return;
		case REPLAY_INPUT_WIN_WHITE:
		case REPLAY_INPUT_WIN_BLACK: {
			EPlayerColour winner = move_result ==
					       REPLAY_INPUT_WIN_WHITE ?
					       COLOUR_WHITE :
					       COLOUR_BLACK;
			if (game_over) {
				printf("Player %d (%s) won!\n", winner + 1,
				       PLAYER_COLOUR_STRINGS[winner]);
			} else {
				printf(
					"Player %d (%s) forfeited, Player %d %s won!\n",
					(winner + 1) % PLAYER_NUM_COLOURS,
					PLAYER_COLOUR_STRINGS[(winner + 1) %
							      PLAYER_NUM_COLOURS],
					winner + 1,
					PLAYER_COLOUR_STRINGS[winner]);
			}
			fclose(file);
			return;
		}
		case REPLAY_INPUT_INVALID:
			INFO_LOG("(INVALID INPUT) %s\n", game->input_buffer);
			fclose(file);
			play_chess(game);
			return;
		case REPLAY_INPUT_EOF:
			INFO_LOG("End of file, replay corrupt or incomplete.\n");
			fclose(file);
			return;
		default:
			break;
		}

		// The parser should have written the next move to the input buffer.
		ECommand command = parse_input(game->input_buffer, game->mode);
		// DEBUG_LOG("Selected command: %s\n", COMMAND_STRINGS[command]);
		switch (command) {
		case COMMAND_MOVE:
			if (move_piece(game, command))
				toggle_player_turn(game);
			continue;
		case COMMAND_QUICK_MOVE:
			if (!select_piece(game))
				continue;
			game->input_pointer = 2;
			game->input_buffer[0] = game->input_buffer[3];
			game->input_buffer[1] = game->input_buffer[4];
			game->input_buffer[2] = '\0';
			if (!move_piece(game, command)) {
				clear_piece_selection(game);
				continue;
			}
			toggle_player_turn(game);
			view_board(game->board, game->selected_piece,
				   game->num_possible_moves,
				   game->possible_moves);
		default:
			break;
		}
	}
	fclose(file);
}
