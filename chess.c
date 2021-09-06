#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdint.h>

#include "game.h"
#include "replay.h"

typedef enum {
    MODE_LOCAL,
    MODE_REPLAY,
    MODE_HOST,
    MODE_JOIN,
    NUM_PROGRAM_MODES
} ProgramMode;

typedef enum {
    PLAYER_HUMAN,
    PLAYER_COMPUTER,
    NUM_PLAYER_TYPES,
} PlayerType;

typedef struct {
    ProgramMode prog_mode;
    PlayerType player1;
    PlayerType player2;
} ChessArgs;

static void init_args(ChessArgs *args) {
    args->prog_mode = MODE_LOCAL;
    args->player1 = PLAYER_HUMAN;
    args->player2 = PLAYER_HUMAN;
}

static bool parse_args(ChessArgs *args, int argc, char **argv) {
    init_args(args);
    if (argc >= 3) {
        if (strncmp(argv[1], "run", 3) == 0) {
            args->prog_mode = MODE_REPLAY;
        }
    }
    return true;
}

int main(int argc, char** argv) {
    static ChessGame game;
    static ChessArgs args;
    // Parse user input.
    if (!parse_args(&args, argc, argv)) {
        printf("Error parsing args etc....");
        return 1;
    }
    // Initialize the chess game, set the board (stage) per se.
    init_chess_game(&game);
    switch (args.prog_mode) {
        case MODE_LOCAL:
            play_chess(&game);
            break;
        case MODE_REPLAY:
            replay_chess(&game, argv[2]);
            break;
        case MODE_HOST:
        case MODE_JOIN:
        default:
            printf("Unimplemented...\n");
            return 0;
    }
    return 0;
}
