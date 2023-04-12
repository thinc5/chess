#include "game.h"
#include "board.h"
#include "display.h"
#include "input.h"
#include "logic.h"
#include "movement.h"
#include "network.h"
#include "pieces.h"
#include "serialization.h"
#include "log.h"

#include <ctype.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

const char *HELP_MESSAGE =
	"'?' or 'help' to display this message, to make a move type\n"
	"the coordinates of the piece to move, and then the target\n"
	"coordinates, e.g 'A1' 'A3'. To forfeit or quit enter 'ff'\n"
	"or 'quit'.\n";

void clear_input_buffer(ChessGame *game)
{
	game->input_pointer = 0;
	memset(game->input_buffer, 0, sizeof(char) * INPUT_BUFFER_SIZE);
}

void init_chess_game(ChessGame *game)
{
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

void toggle_player_turn(ChessGame *game)
{
	game->turn = (game->turn + 1) % PLAYER_NUM_COLOURS;
}

bool select_piece(ChessGame *game)
{
	int selected =
		input_to_index(game->input_buffer[0],
			       game->input_buffer[1]);
	if (game->board[selected].type == PIECE_NONE ||
	    game->board[selected].colour != game->turn) {
		return false;
	}
	game->selected_piece = selected;
	game->num_possible_moves = get_possible_moves_for_piece(
		game->board, game->selected_piece,
		game->possible_moves,
		game->move_count, game->check);
	return true;
}

void clear_piece_selection(ChessGame *game)
{
	game->selected_piece = -1;
	game->num_possible_moves = 0;
	memset(game->possible_moves, 0,
	       sizeof(PossibleMove) * MAX_POSSIBLE_MOVES);
	game->mode = OPERATION_SELECT;
}

bool move_piece(ChessGame *game, ECommand command)
{
	int move_dest = input_to_index(game->input_buffer[0],
				       game->input_buffer[1]);

	// Copy of piece we selected before we modify the board.
	PlayPiece selected_piece = game->board[game->selected_piece];

	// Copy of target piece before we modify the board.
	PlayPiece target_piece = game->board[move_dest];
	int valid_move_index = -1;
	for (size_t i = 0; i < game->num_possible_moves; i++) {
		if (move_dest == game->possible_moves[i].target) {
			valid_move_index = i;
			break;
		}
	}
	if (valid_move_index < 0) {
		INFO_LOG("No valid move for the selected piece!\n");
		return false;
	}

	PossibleMove selected_move = game->possible_moves[valid_move_index];

	// Process the movement.
	process_movement(game->next_board, game->move_count,
			 game->selected_piece,
			 move_dest, game->check);

	// Handle a promotion?
	if (selected_move.type == MOVEMENT_PAWN_PROMOTION) {
		// Special input mode to capture user promotion input.
		game->mode = OPERATION_PROMOTION;
		ECommand promotion_result = COMMAND_INVALID;

		// Piece to modify on successful promotion.
		PlayPiece *promoted_piece = &game->next_board[move_dest];
		while (promotion_result != COMMAND_PROMOTION) {
			show_promotion_prompt(game->turn, move_dest);
			// Reset the buffer.
			memset(game->input_buffer, 0, INPUT_BUFFER_SIZE);
			game->input_pointer = 0;
			// Read a new line.
			read_line(game->input_buffer, &game->input_pointer);
			promotion_result = parse_input(game->input_buffer,
						       game->mode);
		}
		switch (tolower(game->input_buffer[0])) {
		case 'q':
			promoted_piece->type = PIECE_QUEEN;
			break;
		case 'n':
			promoted_piece->type = PIECE_KNIGHT;
			break;
		case 'r':
			promoted_piece->type = PIECE_ROOK;
			break;
		case 'b':
			promoted_piece->type = PIECE_BISHOP;
			break;
		default:
			break;
		}
	}

	// Did the player put themselves in checkmate or are they still in
	// checkmate?
	if (is_checkmate_for_player(game->next_board,
				    game->next_board[move_dest].colour,
				    game->move_count, game->check)) {
		INFO_LOG("Checkmate, move aborted!\n");
		// Reset the board.
		set_board(game->board, game->next_board);
		return false;
	}

	// Write the move to the real board.
	set_board(game->next_board, game->board);
	game->move_count++;

	// Display the result.
	switch (selected_move.type) {
	case MOVEMENT_KING_CASTLE:
		INFO_LOG("Player %d (%s) has castled their %s to %c%c\n",
			 game->turn + 1,
			 PLAYER_COLOUR_STRINGS[game->turn],
			 CHESS_PIECE_STRINGS[selected_piece.type], INT_TO_COORD(
				 move_dest));
		break;
	case MOVEMENT_PIECE_CAPTURE:
		INFO_LOG(
			"Player %d (%s) has moved their %s from %c%c to %c%c and\ncaptured %s's %s\n",
			game->turn + 1,
			PLAYER_COLOUR_STRINGS[game->turn],
			CHESS_PIECE_STRINGS[selected_piece.type], INT_TO_COORD(
				game->selected_piece), INT_TO_COORD(
				move_dest),
			PLAYER_COLOUR_STRINGS[(game->turn + 1) %
					      PLAYER_NUM_COLOURS ],
			PIECE_SYMBOLS[target_piece.colour][target_piece.type]);
		break;
	case MOVEMENT_PAWN_PROMOTION:
		INFO_LOG(
			"Player %d (%s) has promoted their %s to a %s from %c%c to %c%c\n",
			game->turn + 1,
			PLAYER_COLOUR_STRINGS[game->turn],
			CHESS_PIECE_STRINGS[PIECE_PAWN],
			CHESS_PIECE_STRINGS[selected_piece.type], INT_TO_COORD(
				game->selected_piece), INT_TO_COORD(
				move_dest));
		break;
	default:
		INFO_LOG(
			"Player %d (%s) has moved their %s from %c%c to %c%c\n",
			game->turn + 1,
			PLAYER_COLOUR_STRINGS[game->turn],
			CHESS_PIECE_STRINGS[selected_piece.type], INT_TO_COORD(
				game->selected_piece), INT_TO_COORD(
				move_dest));
		break;
	}

	clear_piece_selection(game);
	return true;
}

void play_chess(ChessGame *game)
{
	// Play the game of chess!
	clear_input_buffer(game);
	view_board(game->board, game->selected_piece, game->num_possible_moves,
		   game->possible_moves);

	while (true) {
		game->check = is_checkmate_for_player(game->board, game->turn,
						      game->move_count,
						      game->check);

		// Is the game over?
		if (game->move_count < 0 && game->check) {
			if (!is_game_over_for_player(game->board,
						     game->next_board,
						     game->turn,
						     game->move_count,
						     game->check)) {
				INFO_LOG("Checkmate! player %d (%s) wins!\n",
					 ((game->turn + 1) % PLAYER_NUM_COLOURS) + 1,
					 PLAYER_COLOUR_STRINGS[(game->turn +
								1) %
							       PLAYER_NUM_COLOURS]);
				break;
			}
			INFO_LOG(
				"%s king in check!\n",
				PLAYER_COLOUR_STRINGS[(game->turn) %
						      PLAYER_NUM_COLOURS]);
		}

		// Show the selected piece if we have one.
		EChessPiece type = game->selected_piece == -1
			  ? PIECE_NONE
			  : game->board[game->selected_piece].type;
		show_possible_moves(game->selected_piece, type,
				    game->num_possible_moves,
				    game->possible_moves);

		// Get some input.
		show_prompt(game->turn, type);
		// Reset the buffer.
		memset(game->input_buffer, 0, INPUT_BUFFER_SIZE);
		game->input_pointer = 0;
		// Read a new line.
		read_line(game->input_buffer, &game->input_pointer);
		// See if the input was syntactically valid.
		ECommand command = parse_input(game->input_buffer, game->mode);
		switch (command) {
		case COMMAND_SAVE:
			INFO_LOG("Saving game...\n");
			if (!serialize(game, game->input_buffer)) {
				INFO_LOG("Unable to save game...\n");
				continue;
			}
			INFO_LOG("Game saved...\n");
			continue;
		case COMMAND_LOAD:
			INFO_LOG("Loading game...\n");
			if (!deserialize(game, game->input_buffer)) {
				INFO_LOG("Unable to load game...\n");
				continue;
			}
			INFO_LOG("Game loaded...\n");
			view_board(game->board, game->selected_piece,
				   game->num_possible_moves,
				   game->possible_moves);
			continue;
		case COMMAND_FORFEIT:
			INFO_LOG("%s player forfeits, %s player wins!\n",
				 PLAYER_COLOUR_STRINGS[game->turn],
				 PLAYER_COLOUR_STRINGS[(game->turn + 1) %
						       PLAYER_NUM_COLOURS]);
			return;
		case COMMAND_HELP:
			INFO_LOG("%s", HELP_MESSAGE);
			continue;
		case COMMAND_SELECT: {
			if (!select_piece(game))
				continue;
			view_board(game->board, game->selected_piece,
				   game->num_possible_moves,
				   game->possible_moves);
			game->mode = OPERATION_MOVE;
			continue;
		}
		case COMMAND_CLEAR:
			clear_piece_selection(game);
			continue;
		case COMMAND_MOVE:
			if (move_piece(game, command))
				toggle_player_turn(game);
			view_board(game->board, game->selected_piece,
				   game->num_possible_moves,
				   game->possible_moves);
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
			view_board(game->board, game->selected_piece,
				   game->num_possible_moves,
				   game->possible_moves);
			toggle_player_turn(game);
			continue;
		case COMMAND_INVALID:
		default:
			INFO_LOG("Invalid command...\n");
			continue;
		}
	}
}

void play_chess_networked(EGameMode mode, ChessGame *game,
			  int connection_fd)
{
	// Play the game of chess!
	view_board(game->board, game->selected_piece, game->num_possible_moves,
		   game->possible_moves);

	while (true) {
		game->check = is_checkmate_for_player(game->board, game->turn,
						      game->move_count,
						      game->check);

		// Is the game over?
		if (game->check) {
			if (!is_game_over_for_player(game->board,
						     game->next_board,
						     game->turn,
						     game->move_count,
						     game->check)) {
				INFO_LOG("Checkmate! player %d (%s) wins!\n",
					 ((game->turn + 1) % PLAYER_NUM_COLOURS) + 1,
					 PLAYER_COLOUR_STRINGS[(game->turn +
								1) %
							       PLAYER_NUM_COLOURS]);
				break;
			}
			INFO_LOG(
				"%s king in check!\n",
				PLAYER_COLOUR_STRINGS[(game->turn) %
						      PLAYER_NUM_COLOURS]);
			// Check if the game is in stalemate since it is not in
			// check
		} else if (!is_game_stalemate(game->board, game->turn,
					      game->move_count)) {
			INFO_LOG("Stalemate! Game ends in draw!\n");
			break;
		}

		// Show the selected piece if we have one.
		EChessPiece type = game->selected_piece == -1
			  ? PIECE_NONE
			  : game->board[game->selected_piece].type;
		show_possible_moves(game->selected_piece, type,
				    game->num_possible_moves,
				    game->possible_moves);

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
			INFO_LOG("waiting for other player's turn\n");
			game->input_pointer =
				read_network_line(connection_fd,
						  game->input_buffer);
			if (game->input_pointer == 0) {
				INFO_LOG("No input from other player, error\n");
				return;
			}
			if (game->input_pointer == -1) {
				INFO_LOG("Player pipe broke, error\n");
				return;
			}
			INFO_LOG("Other player's command: %s\n",
				 game->input_buffer);
		}
		// See if the input was syntactically valid.
		ECommand command = parse_input(game->input_buffer, game->mode);
		switch (command) {
		case COMMAND_SAVE:
			INFO_LOG("Saving game...\n");
			if (!serialize(game, "save.bin")) {
				INFO_LOG("Unable to save game...\n");
				continue;
			}
			INFO_LOG("Game saved...\n");
			continue;
		case COMMAND_FORFEIT:
			if (game->turn == game->player) {
				write_network_line(connection_fd,
						   game->input_buffer,
						   (int)game->input_pointer);
			}
			INFO_LOG("Player %s forfeits!\n",
				 PLAYER_COLOUR_STRINGS[game->turn]);
			return;
		case COMMAND_HELP:
			INFO_LOG("%s", HELP_MESSAGE);
			continue;
		case COMMAND_SELECT: {
			int selected =
				input_to_index(game->input_buffer[0],
					       game->input_buffer[1]);
			if (game->board[selected].type == PIECE_NONE ||
			    game->board[selected].colour != game->turn) {
				continue;
			}
			game->selected_piece = selected;
			game->num_possible_moves = get_possible_moves_for_piece(
				game->board, game->selected_piece,
				game->possible_moves,
				game->move_count, game->check);
			view_board(game->board, game->selected_piece,
				   game->num_possible_moves,
				   game->possible_moves);
			game->mode = OPERATION_MOVE;
			if (game->turn == game->player) {
				write_network_line(connection_fd,
						   game->input_buffer,
						   (int)game->input_pointer);
			}
			continue;
		}
		case COMMAND_CLEAR:
			clear_piece_selection(game);
			if (game->turn == game->player) {
				write_network_line(connection_fd,
						   game->input_buffer,
						   (int)game->input_pointer);
			}
			view_board(game->board, game->selected_piece,
				   game->num_possible_moves,
				   game->possible_moves);
			continue;
		case COMMAND_MOVE:
			if (move_piece(game, command)) {
				INFO_LOG("Successfully moved piece!\n");
				if (game->turn == game->player) {
					write_network_line(connection_fd,
							   game->input_buffer,
							   (int)game->input_pointer);
				}
				toggle_player_turn(game);
			}
			;
			continue;
		case COMMAND_INVALID:
		default:
			INFO_LOG("Invalid command...\n");
			continue;
		}
	}
}
