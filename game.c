#include <stddef.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include "pieces.h"
#include "board.h"
#include "movement.h"
#include "logic.h"
#include "input.h"
#include "display.h"
#include "serialization.h"
#include "network.h"
#include "game.h"

const char *HELP_MESSAGE = 
        "? or help to display this message, to make a move type\n"                
        "the coordinates of the piece to move, and then the\n"
        "target coordinates, e.g A1 A3. To FORFEITor quit enter\n"
        "ff or quit.\n";

void clear_piece_selection(ChessGame *game) {
    game->selected_piece = -1;
    game->num_possible_moves = 0;
    memset(game->possible_moves, 0, sizeof(PossibleMove) * MAX_POSSIBLE_MOVES);
    game->mode = OPERATION_SELECT;
}

void clear_input_buffer(ChessGame *game) {
    game->input_pointer = 0;
    memset(game->input_buffer, 0, sizeof(char) * INPUT_BUFFER_SIZE);
}

void init_chess_game(ChessGame *game) {
    // Initialize the boards.
    new_board(game->board);
    set_board(game->board, game->next_board);
    // Set the starting player.
    game->player = COLOUR_WHITE;
    game->turn = COLOUR_WHITE;
    // Start with 0 moves!
    game->move_count = 0;
    // Set the operation mode.
    game->mode = OPERATION_SELECT;
    // No piece selected.
    clear_piece_selection(game);
    // Input parsing.
    clear_input_buffer(game);
}

void toggle_player_turn(ChessGame *game) {
    game->turn = (game->turn + 1) % NUM_PLAYER_COLOURS;
}

bool move_piece(ChessGame *game, Command command) {
    int move_dest = input_to_index(game->input_buffer[0], game->input_buffer[1]);
    int valid_move = -1;
    for (size_t i = 0; i < game->num_possible_moves; i++) {
        if (move_dest == game->possible_moves[i].target) {
            valid_move = i;
            break;
        }
    }
    if (valid_move < 0) {
        printf("NO valid move!\n");
        return false;
    }

    process_movement(game->next_board, game->move_count, game->selected_piece, move_dest, game->check);
    PlayPiece *selected_piece = &game->next_board[game->selected_piece];

    // Handle a promotion?
    if (game->possible_moves[valid_move].type == MOVEMENT_PAWN_PROMOTION) {
        game->mode = OPERATION_PROMOTION;
        Command promotion = COMMAND_INVALID;
        while (promotion != COMMAND_PROMOTION) {
            show_promotion_prompt(game->turn, move_dest);
            // Reset the buffer.
            memset(game->input_buffer, 0, INPUT_BUFFER_SIZE);
            game->input_pointer = 0;
            // Read a new line.
            read_line(game->input_buffer, &game->input_pointer);
            promotion = parse_input(game->input_buffer, game->mode);
        }
        switch(tolower(game->input_buffer[0])) {
            case 'q':
                selected_piece->type = PIECE_QUEEN;
                break;
            case 'k':
                selected_piece->type = PIECE_KNIGHT;
                break;
            case 'r':
                selected_piece->type = PIECE_ROOK;
                break;
            case 'b':
                selected_piece->type = PIECE_BISHOP;
                break;
            default:
                break;
        }
    }

    // Did the player put themselves in checkmate or are they still in checkmate?
    if (is_checkmate_for_player(game->next_board, game->next_board[move_dest].colour, game->move_count, game->check)) {
        printf("Checkmate, move aborted!\n");
        set_board(game->board, game->next_board);
        return false;
    }

    // Update the real board.
    set_board(game->next_board, game->board);
    game->move_count++;
    printf("game count: %ld\n", game->move_count);
    printf("Player %d (%s) has moved their %s to %c%c\n",
            game->turn + 1, PLAYER_COLOUR_STRINGS[game->turn],
            CHESS_PIECE_STRINGS[selected_piece->type],
            INT_TO_COORD(move_dest));

    clear_piece_selection(game);
    view_board(game->board, game->selected_piece, game->num_possible_moves, game->possible_moves);
    return true;
}

void play_chess(ChessGame *game) {
    // Play the game of chess!
    view_board(game->board, game->selected_piece, game->num_possible_moves, game->possible_moves);

    while (true) {
        game->check = is_checkmate_for_player(game->board, game->turn, game->move_count, game->check);

        // Is the game over?
        if (game->move_count < 0 && game->check) {
            if (!is_game_over_for_player(game->board, game->next_board, game->turn, game->move_count, game->check)) {
                printf("Checkmate! player %d (%s) wins!\n", ((game->turn + 1) % NUM_PLAYER_COLOURS) + 1,
                        PLAYER_COLOUR_STRINGS[(game->turn + 1) % NUM_PLAYER_COLOURS]);
                break;
            }
            printf("%s king located in check!\n", PLAYER_COLOUR_STRINGS[((game->turn + 1) % NUM_PLAYER_COLOURS) + 1]);
        }

        // Show the selected piece if we have one.
        ChessPiece type = game->selected_piece == -1 ? PIECE_NONE : game->board[game->selected_piece].type;
        show_possible_moves(game->selected_piece, type, game->num_possible_moves, game->possible_moves);

        // Get some input.
        show_prompt(game->turn, type);
        // Reset the buffer.
        memset(game->input_buffer, 0, INPUT_BUFFER_SIZE);
        game->input_pointer = 0;
        // Read a new line.
        read_line(game->input_buffer, &game->input_pointer);
        // See if the input was syntactically valid.
        Command command = parse_input(game->input_buffer, game->mode);
        switch (command) {
            case COMMAND_SAVE:
                printf("Saving game...\n");
                if (!serialize(game, "save.bin")) {
                    printf("Unable to save game...\n");
                    continue;
                }
                printf("Game saved...\n");
                continue;
            case COMMAND_LOAD:
                printf("Loading game...\n");
                if (!deserialize(game, "save.bin")) {
                    printf("Unable to load game...\n");
                    continue;
                }
                printf("Game loaded...\n");
                view_board(game->board, game->selected_piece, game->num_possible_moves, game->possible_moves);
                continue;
            case COMMAND_FORFEIT:
                printf("Player %d FORFEITs, Player %d wins!\n", game->turn + 1, game->turn);
                return;
            case COMMAND_HELP:
                printf("%s", HELP_MESSAGE);
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
                    continue;
                }
            case COMMAND_CLEAR:
                clear_piece_selection(game);
                continue;
            case COMMAND_MOVE:
                move_piece(game, command);
                continue;
            case COMMAND_INVALID:
            default:
                printf("Invalid command...\n");
                continue;
        }
    }
}

void play_chess_networked(ProgramMode mode, ChessGame *game, int connection_fd) {
    // Play the game of chess!
    view_board(game->board, game->selected_piece, game->num_possible_moves, game->possible_moves);

    while (true) {
        game->check = is_checkmate_for_player(game->board, game->turn, game->move_count, game->check);

        // Is the game over?
        if (game->check) {
            if (!is_game_over_for_player(game->board, game->next_board, game->turn, game->move_count, game->check)) {
                printf("Checkmate! player %d (%s) wins!\n", ((game->turn + 1) % NUM_PLAYER_COLOURS) + 1,
                        PLAYER_COLOUR_STRINGS[(game->turn + 1) % NUM_PLAYER_COLOURS]);
                break;
            }
            printf("%s king located in check!\n", PLAYER_COLOUR_STRINGS[((game->turn + 1) % NUM_PLAYER_COLOURS) + 1]);
        // Check if the game is in stalemate since it is not in check
        } else if (!is_game_stalemate(game->board, game->turn, game->move_count)) {
                printf("Stalemate! Game ends in draw!\n");
                break;
        }

        // Show the selected piece if we have one.
        ChessPiece type = game->selected_piece == -1 ? PIECE_NONE : game->board[game->selected_piece].type;
        show_possible_moves(game->selected_piece, type, game->num_possible_moves, game->possible_moves);

        // Get some input.
        // Reset the buffer.
        memset(game->input_buffer, 0, INPUT_BUFFER_SIZE);
        game->input_pointer = 0;
        // Read a new line.
        if (game->turn == game->player) {
            // From user this user.
            show_prompt(game->turn, type);
            read_line(game->input_buffer, &game->input_pointer);
        } else {
            // From connection (other player).
            printf("waiting for other player's turn\n");
            game->input_pointer = read_network_line(connection_fd, game->input_buffer);
            if (game->input_pointer == 0) {
                printf("No input from other player, error\n");
                return;
            }
            if (game->input_pointer == -1) {
                printf("Player pipe broke, error\n");
                return;
            }
            printf("Other player's command: %s\n", game->input_buffer);
        }
        // See if the input was syntactically valid.
        Command command = parse_input(game->input_buffer, game->mode);
        switch (command) {
            case COMMAND_SAVE:
                printf("Saving game...\n");
                if (!serialize(game, "save.bin")) {
                    printf("Unable to save game...\n");
                    continue;
                }
                printf("Game saved...\n");
                continue;
            case COMMAND_LOAD:
                printf("Loading game...\n");
                if (!deserialize(game, "save.bin")) {
                    printf("Unable to load game...\n");
                    continue;
                }
                printf("Game loaded...\n");
                view_board(game->board, game->selected_piece, game->num_possible_moves, game->possible_moves);
                continue;
            case COMMAND_FORFEIT:
                if (game->turn == game->player) {
                    write_network_line(connection_fd, game->input_buffer, (int) game->input_pointer);
                }
                printf("Player %s forfeits!\n", PLAYER_COLOUR_STRINGS[game->turn]);
                return;
            case COMMAND_HELP:
                printf("%s", HELP_MESSAGE);
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
                    if (game->turn == game->player) {
                        write_network_line(connection_fd, game->input_buffer, (int) game->input_pointer);
                    }
                    continue;
                }
            case COMMAND_CLEAR:
                clear_piece_selection(game);
                if (game->turn == game->player) {
                    write_network_line(connection_fd, game->input_buffer, (int) game->input_pointer);
                }
                view_board(game->board, game->selected_piece, game->num_possible_moves, game->possible_moves);
                continue;
            case COMMAND_MOVE:
                if (move_piece(game, command)) {
                    printf("Successfully moved piece!\n");
                    if (game->turn == game->player) {
                        write_network_line(connection_fd, game->input_buffer, (int) game->input_pointer);
                    }
                    toggle_player_turn(game);
                };
                continue;
            case COMMAND_INVALID:
            default:
                printf("Invalid command...\n");
                continue;
        }
    }
}
