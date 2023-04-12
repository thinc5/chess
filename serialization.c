#include "serialization.h"
#include "game.h"
#include "pieces.h"
#include "display.h"
#include "log.h"

#include <stdio.h>
#include <string.h>
#include <wchar.h>

typedef struct {
	// Stores player and turn.
	char player_info[3];
	char move_count[6];
	char board[BOARD_SIZE * BOARD_SIZE * 2];
} ChessGameText;

char *default_filepath = "save.bin";

#define LOCAL_BUFFER_SIZE 255
static char local_buffer[LOCAL_BUFFER_SIZE];

// Raw processor/os specific binary dump.
int serialize_raw(ChessGame *game, FILE *file)
{
	if (fwrite(game, sizeof(ChessGame), 1, file) != 1) {
		return 0;
	}
	return 1;
}

int deserialize_raw(ChessGame *game, FILE *file)
{
	if (fread(&game, sizeof(ChessGame), 0, file) != 1) {
		return 0;
	}
	return 1;
}

int serialize_text(ChessGame *game, FILE *file)
{
	// Game data.
	if (fputc(game->player + '0',
		  file) == EOF) {
		return 0;
	}

	if (fputc(game->turn + '0',
		  file) == EOF) {
		return 0;
	}

	if (fputc('\n', file) == EOF) {
		return 0;
	}

	snprintf(local_buffer, 255, "%lu", game->move_count);
	if (fputs(local_buffer, file) == EOF) {
		return 0;
	}

	if (fputc('\n', file) == EOF) {
		return 0;
	}

	// Board.
	for (int i = 0; i < BOARD_SIZE * BOARD_SIZE; i++) {
		PlayPiece piece = game->board[i];
		if (fputs(PIECE_SYMBOLS[piece.colour][piece.type],
			  file) == EOF) {
			return 0;
		}

		if (i != 0 && (i % (BOARD_SIZE)) == BOARD_SIZE - 1) {
			if (fputc('\n', file) == EOF) {
				return 0;
			}
		}
	}
	return 1;
}

static bool identify_piece_sym(const char *str, PlayPiece *piece)
{
	for (PlayerColour colour = COLOUR_WHITE; colour < NUM_PLAYER_COLOURS;
	     colour++) {
		for (ChessPiece pieceType = PIECE_NONE;
		     pieceType < NUM_CHESS_PIECES; pieceType++) {
			if (strcmp(str,
				   PIECE_SYMBOLS[colour][pieceType]) == 0) {
				*piece =
					(PlayPiece) { .colour = colour,
						      .type = pieceType,
						      .moves = 0,
						      .last_move = 0 };
				return true;
			}
		}
	}
	return false;
}

int deserialize_text(ChessGame *game, FILE *file)
{
	char _read;
	// Game data.

	if ((_read = fgetc(
		     file)) == EOF) {
		return 0;
	}
	game->player = _read - '0';

	if ((_read = fgetc(
		     file)) == EOF) {
		return 0;
	}
	game->turn = _read - '0';

	// Expect a newline.
	_read = fgetc(
		file);
	if (_read == EOF || _read != '\n') {
		return 0;
	}

	// Move count.
	for (int i = 0; i < LOCAL_BUFFER_SIZE; i++) {
		_read = fgetc(file);
		if (_read == EOF) {
			return 0;
		}
		if (_read == '\n') {
			break;
		}
		local_buffer[i] = _read;
	}
	game->move_count = atol(local_buffer);
	memset(local_buffer, '\0', LOCAL_BUFFER_SIZE * sizeof(char));

	// Board.
	for (int i = 0; i < BOARD_SIZE * BOARD_SIZE; i++) {
		PlayPiece *piece = &game->board[i];
		memset(local_buffer, '\0', LOCAL_BUFFER_SIZE * sizeof(char));
		// Can only read as many characters as would fit into our buffer.
		for (size_t j = 0; j < LOCAL_BUFFER_SIZE; j++) {
			_read = fgetc(file);
			// End of file early, invalid save.
			if (_read == EOF) {
				DEBUG_LOG("deserialize_text eof\n");
				return 0;
			}
			local_buffer[j] = _read;

			// Process the string, do we have something?
			if (strcmp(local_buffer, "\n") == 0) {
				// We need to be at the end of a row to expect a newline.
				if (i % 8 != 0) {
					return 0;
				}
				i--;
				break;
			}

			// Process the string, do we have something?
			if (identify_piece_sym(local_buffer, piece)) {
				break;
			}
		}
	}
	memcpy(game->next_board, game->board, sizeof(Board));
	return 1;
}

char *get_filepath(char *command)
{
	char *path = strtok(command, " ");
	path = strtok(NULL, " ");
	DEBUG_LOG("PATH: %s\n", path);
	return path;
}

int serialize(ChessGame *game, char *command)
{
	char *filepath = get_filepath(command);
	char *selected = filepath ? filepath : default_filepath;
	FILE *file = fopen(filepath, "w");
	if (file == NULL) {
		return 0;
	}
	if (!serialize_text(game, file)) {
		return 0;
	}
	fclose(file);
	INFO_LOG("Saved to %s\n", selected);
	return 1;
}

int deserialize(ChessGame *game, char *command)
{
	ChessGame local = { 0 };
	init_chess_game(&local);
	char *filepath = get_filepath(command);
	char *selected = filepath ? filepath : default_filepath;;
	FILE *file = fopen(selected, "r");
	if (file == NULL) {
		return 0;
	}
	if (!deserialize_text(&local, file)) {
		return 0;
	}
	fclose(file);
	memcpy(game, &local, sizeof(ChessGame));
	INFO_LOG("Loaded %s\n", selected);
	return 1;
}
