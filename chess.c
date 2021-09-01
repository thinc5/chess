#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdint.h>

#include "game.h"

typedef enum {
    PLAYER_HUMAN,
    PLAYER_COMPUTER,
    NUM_PLAYER_TYPES,
} PlayerType;

typedef struct {
    PlayerType player1;
    PlayerType player2;
} ChessArgs;

bool parse_args(ChessArgs* args, int argc, char** argv) {
    args->player1 = PLAYER_HUMAN;
    args->player2 = PLAYER_HUMAN;
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
    // Initialize the chess game.
    init_chess_game(&game);
    play_chess(&game);
    return 0;
}
