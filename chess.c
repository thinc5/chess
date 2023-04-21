#include "core/game.h"
#include "core/network.h"
#include "core/replay.h"
#include "core/serialization.h"
#include "core/log.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

const char *GAME_MODE_COMMANDS[] = {
	[GAME_MODE_LOCAL] = "local", [GAME_MODE_LOAD] = "load",
	[GAME_MODE_REPLAY] = "replay", [GAME_MODE_HOST] = "host",
	[GAME_MODE_JOIN] = "join"
};

static void init_args(ChessArgs *args)
{
	args->prog_mode = GAME_MODE_INVALID;
	args->player1 = PLAYER_HUMAN;
	args->player2 = PLAYER_HUMAN;
}

static void parse_args(ChessArgs *args, int argc, char **argv)
{
	init_args(args);
	if (argc == 1 ||
	    (argc == 2 && strncmp(argv[1], GAME_MODE_COMMANDS[GAME_MODE_LOCAL],
				  strlen(GAME_MODE_COMMANDS[
						 GAME_MODE_LOCAL])) == 0)) {
		args->prog_mode = GAME_MODE_LOCAL;
	} else if (argc == 3 &&
		   strncmp(argv[1], GAME_MODE_COMMANDS[GAME_MODE_LOAD],
			   strlen(GAME_MODE_COMMANDS[GAME_MODE_LOAD]))
		   == 0) {
		args->prog_mode = GAME_MODE_LOAD;
	} else if (argc == 3 &&
		   strncmp(argv[1], GAME_MODE_COMMANDS[GAME_MODE_REPLAY],
			   strlen(GAME_MODE_COMMANDS[GAME_MODE_REPLAY]))
		   == 0) {
		args->prog_mode = GAME_MODE_REPLAY;
	} else if ((argc == 2 || argc == 3 || argc == 4) &&
		   strncmp(argv[1], GAME_MODE_COMMANDS[GAME_MODE_HOST],
			   strlen(GAME_MODE_COMMANDS
				  [GAME_MODE_HOST])) == 0) {
		args->prog_mode = GAME_MODE_HOST;
	} else if ((argc == 3 || argc == 4) &&
		   strncmp(argv[1], GAME_MODE_COMMANDS[GAME_MODE_JOIN],
			   strlen(GAME_MODE_COMMANDS
				  [GAME_MODE_JOIN])) == 0
		   ) {
		args->prog_mode = GAME_MODE_JOIN;
	}
}

int main(int argc, char **argv)
{
	static ChessGame game;
	static ChessArgs args;
	static int connection_fd;

	// Parse user input.
	parse_args(&args, argc, argv);

	// Initialize the chess game, set the board (stage) per se.
	init_chess_game(&game);
	DEBUG_LOG("Running mode %s\n", GAME_MODE_COMMANDS[args.prog_mode]);

	switch (args.prog_mode) {
	case GAME_MODE_LOCAL:
		play_chess(&game);
		break;
	case GAME_MODE_LOAD:
		snprintf(game.input_buffer, INPUT_BUFFER_SIZE, "%s %s",
			 argv[1], argv[2]);
		if (!deserialize(&game, game.input_buffer)) {
			INFO_LOG("Unable to load game: %s\n", argv[2]);
			break;
		}
		play_chess(&game);
		break;
	case GAME_MODE_REPLAY:
		replay_chess(&game, argv[2]);
		break;
	case GAME_MODE_HOST:
		// Port is optional.
		connection_fd = host_server(argc < 3 ? NULL : argv[2]);
		if (connection_fd == 0) {
			return 0;
		}
		play_chess_networked(args.prog_mode, &game, connection_fd);
		close(connection_fd);
		break;
	case GAME_MODE_JOIN:
		connection_fd = connect_to_sever(argv[2], argv[3]);
		if (connection_fd == 0) {
			return 0;
		}
		game.player = COLOUR_BLACK;
		play_chess_networked(args.prog_mode, &game, connection_fd);
		close(connection_fd);
		break;
	default:
		INFO_LOG("Error parsing args etc....\n");
		return 0;
	}

	return 0;
}
