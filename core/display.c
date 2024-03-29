#include "display.h"
#include "board.h"
#include "log.h"

#include <stdbool.h>
#include <stdio.h>


const char INPUT_PROMPT_SYMBOL = '>';

void debug_show_piece(PlayPiece piece)
{
	if (piece.type != PIECE_NONE)
		DEBUG_LOG("%s (%s %s)\n",
			  PIECE_SYMBOLS[piece.colour][piece.
						      type],
			  PLAYER_COLOUR_STRINGS[piece.
						colour],
			  CHESS_PIECE_STRINGS[piece.type]);
	else
		DEBUG_LOG("No piece\n");
}

static inline void print_x_axis(void)
{
	printf(" |");
	for (int col = 0; col < BOARD_SIZE; col++)
		printf("%c|", col + 'A');
	printf("\n");
}

void view_board(Board board, int selected, size_t possible_moves,
		PossibleMove possible[MAX_POSSIBLE_MOVES])
{
	print_x_axis();
	for (int row = BOARD_SIZE; row > 0; row--) {
		// Y axis.
		printf("%d|", row);
		// Print the piece.
		for (int col = 0; col < BOARD_SIZE; col++) {
			// Do we have a target at this location?
			bool matched = false;
			for (int i = 0; i < possible_moves; i++) {
				if (possible[i].target ==
				    ((row - 1) * BOARD_SIZE) + col) {
					switch (possible[i].type) {
					case MOVEMENT_NORMAL:
					case MOVEMENT_PAWN_LONG_JUMP:
						printf(".");
						break;
					case MOVEMENT_PIECE_CAPTURE:
						printf("x");
						break;
					case MOVEMENT_PAWN_PROMOTION:
						printf("P");
						break;
					case MOVEMENT_PAWN_EN_PASSANT:
						printf("e");
						break;
					case MOVEMENT_KING_CASTLE:
						printf("c");
						break;
					case MOVEMENT_ILLEGAL:
					default:
						printf(" ");
						break;
					}
					matched = true;
					break;
				}
			}
			// Else print this.
			if (!matched) {
				PlayPiece piece =
					board[((row - 1) * BOARD_SIZE) + col];
				printf("%s",
				       PIECE_SYMBOLS[piece.colour][piece.type]);
			}
			printf("|");
		}
		// Y axis.
		printf("%d", row);
		printf("\n");
	}
	print_x_axis();
}

void show_prompt(EPlayerColour turn, EChessPiece piece)
{
	printf("P%d (%s)", turn + 1, PLAYER_COLOUR_STRINGS[turn]);
	if (piece != PIECE_NONE)
		printf(" %s", PIECE_SYMBOLS[turn][piece]);
	printf(" %c ", INPUT_PROMPT_SYMBOL);
	fflush(stdin);
}

void show_promotion_prompt(EPlayerColour turn, int selected)
{
	printf("Promotion for %s at %c%c; Q (%s), N (%s), R (%s), B (%s)? %c ",
	       PIECE_SYMBOLS[turn][PIECE_PAWN], INT_TO_COORD(
		       selected),
	       PIECE_SYMBOLS[turn][PIECE_QUEEN],
	       PIECE_SYMBOLS[turn][PIECE_KNIGHT],
	       PIECE_SYMBOLS[turn][PIECE_ROOK], PIECE_SYMBOLS[turn][PIECE_BISHOP],
	       INPUT_PROMPT_SYMBOL);
}

void show_question_prompt(const char *question)
{
	printf("%s y/N %c\n", question, INPUT_PROMPT_SYMBOL);
}

void show_possible_moves(int selected, EChessPiece piece, size_t possible_moves,
			 PossibleMove possible[MAX_POSSIBLE_MOVES])
{
	if (selected < 0) {
		return;
	}
	if (possible_moves < 1) {
		printf("No possible moves for selected %s\n",
		       CHESS_PIECE_STRINGS[piece]);
		return;
	}
	printf("Possible moves for %s (%c%c):", CHESS_PIECE_STRINGS[piece],
	       INT_TO_COORD(selected));
	for (int i = 0; i < possible_moves; i++) {
		printf(" %c%c (%d) (type: %s)", INT_TO_COORD(
			       possible[i].target),
		       possible[i].target,
		       MOVEMENT_TYPE_STRINGS[possible[i].type]);
	}
	printf("\n");
}
