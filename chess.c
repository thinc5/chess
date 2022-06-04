#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "game.h"
#include "network.h"
#include "replay.h"

static void
init_args(ChessArgs *args)
{
    args->prog_mode = MODE_LOCAL;
    args->player1   = PLAYER_HUMAN;
    args->player2   = PLAYER_HUMAN;
}

static bool
parse_args(ChessArgs *args, int argc, char **argv)
{
    init_args(args);
    switch (argc)
        {
        case 1:
            return true;
        case 3:
            if (strncmp(argv[1], "run", 3) == 0 && argc == 3)
                {
                    args->prog_mode = MODE_REPLAY;
                }
            else if (strncmp(argv[1], "host", 4) == 0
                     && (argc == 2 || argc == 3))
                {
                    args->prog_mode = MODE_HOST;
                }
            else if (strncmp(argv[1], "join", 4) == 0
                     && (argc == 3 || argc == 4))
                {
                    args->prog_mode = MODE_JOIN;
                }
            else
                {
                    return false;
                }
            return true;
        default:
            return false;
        }
}

int
main(int argc, char **argv)
{
    static ChessGame game;
    static ChessArgs args;
    static int connection_fd;
    // Parse user input.
    if (!parse_args(&args, argc, argv))
        {
            printf("Error parsing args etc....\n");
            return 1;
        }
    // Initialize the chess game, set the board (stage) per se.
    init_chess_game(&game);
    switch (args.prog_mode)
        {
        case MODE_LOCAL:
            play_chess(&game);
            break;
        case MODE_REPLAY:
            replay_chess(&game, argv[2]);
            break;
        case MODE_HOST:
            connection_fd = host_server(argv[2]);
            if (connection_fd == 0) { return 0; }
            play_chess_networked(args.prog_mode, &game, connection_fd);
            close(connection_fd);
            break;
        case MODE_JOIN:
            connection_fd = connect_to_sever(argv[2], argv[3]);
            if (connection_fd == 0) { return 0; }
            game.player = COLOUR_BLACK;
            play_chess_networked(args.prog_mode, &game, connection_fd);
            close(connection_fd);
            break;
        default:
            printf("Unimplemented...\n");
            return 0;
        }
    return 0;
}
