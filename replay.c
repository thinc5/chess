#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "game.h"
#include "logic.h"
#include "display.h"

#include "replay.h"

typedef enum {
    FORMAT_INVALID,
    FORMAT_RAW,
    FORMAT_PGN,
    NUM_FILE_FORMATS,
} FileFormat;

void replay_chess(ChessGame *game, const char *filepath) {
    // Does file exist?
    FILE *file = fopen(filepath, "r");
    if (file == NULL) {
        printf("File does not exist or cannot be read.");
        return;
    }

    // Read first line and determine the read mode.
    FileFormat format = FORMAT_INVALID;
    memset(game->input_buffer, 0, INPUT_BUFFER_SIZE);
    game->input_pointer = 0;
    while ((c = fgetc(file))) {
        if (c == EOF || c == '\n') {
            break;
        }
        game->input_buffer[game->input_pointer] = c;
        game->input_pointer++;
    }
    if (strncmp(game->input_buffer, "raw", 3) == 0) {
        format = FORMAT_RAW;
    } else if (strncmp(game->input_buffer, "pgn", 3) == 0) {
        format = FORMAT_PGN;
    }

    view_board(game->board, game->selected_piece, game->num_possible_moves, game->possible_moves);
    while (true) {
        // Is the game over?
        if (game->move_count > 0) {
            game->check = is_checkmate_for_player(game->board, game->turn, game->move_count, game->check);
            if (game->check) {
                if (!is_game_over_for_player(game->board, game->next_board, game->turn, game->move_count, game->check)) {
                    printf("Checkmate! player %d (%s) wins with %ld moves!\n", ((game->turn + 1) % NUM_PLAYER_COLOURS) + 1,
                            PLAYER_COLOUR_STRINGS[(game->turn + 1) % NUM_PLAYER_COLOURS], game->move_count);
                    break;
                }
                printf("%s king located in check!\n", PLAYER_COLOUR_STRINGS[((game->turn + 1) % NUM_PLAYER_COLOURS) + 1]);
            }
        }

        // Show the selected piece if we have one.
        // ChessPiece type = game->selected_piece == -1 ? PIECE_NONE : game->board[game->selected_piece].type;
        // show_possible_moves(game->selected_piece, type, game->num_possible_moves, game->possible_moves);

        // Read file line by line and throw out any malformed content until eof.
        memset(game->input_buffer, 0, INPUT_BUFFER_SIZE);
        game->input_pointer = 0;

        bool comment = false;
        char c = 0;
        while ((c = fgetc(file))) {
            if (game->input_pointer == 0 && c == '#') {
                comment = true;
            }
            if (c == EOF || c == '\n') {
                break;
            }
            if (c == ' ') {
                continue;
            }
            game->input_buffer[game->input_pointer] = c;
            game->input_pointer++;
        }
        if (comment) {
            continue;
        }
        printf("%s (%ld)\n", game->input_buffer, game->input_pointer);
        Command command = parse_input(game->input_buffer, game->mode);
        printf("Selected command: %s\n", COMMAND_STRINGS[command]);
        switch (command) {
            case COMMAND_MOVE:
                move_piece(game, command);
                continue;
            case COMMAND_SELECT: {
                    int selected = input_to_index(game->input_buffer[0], game->input_buffer[1]);
                    if (game->board[selected].type == PIECE_NONE || game->board[selected].colour != game->turn) {
                        continue;
                    }
                    game->selected_piece = selected;
                    game->num_possible_moves = get_possible_moves_for_piece(game->board, game->selected_piece, game->possible_moves, game->move_count, game->check);
                    view_board(game->board, game->selected_piece, game->num_possible_moves, game->possible_moves);
                    game->mode = OPERATION_MOVE;
                    // show_possible_moves(game->selected_piece, game->board[game->selected_piece].type, game->num_possible_moves, game->possible_moves);
                    continue;
                }
                break;
            default:
                break;
        }

        if (c == EOF) {
            printf("Reached EOF, replay over..\n");
            break;
        }
    }
    fclose(file);
}
