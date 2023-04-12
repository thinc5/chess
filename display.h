#ifndef _DISPLAY_H
#define _DISPLAY_H

#include "movement.h"
#include "pieces.h"

#define INT_TO_COORD(position) \
	(position % BOARD_SIZE) + 'A', (position / BOARD_SIZE) + '1'


void debug_show_piece(PlayPiece piece);

void view_board(Board board, int selected, size_t possible_moves,
		PossibleMove possible[MAX_POSSIBLE_MOVES]);
void show_prompt(PlayerColour turn, ChessPiece piece);
void show_promotion_prompt(PlayerColour turn, int selected);
void show_question_prompt(const char *question);
void show_possible_moves(int selected, ChessPiece piece, size_t possible_moves,
			 PossibleMove possible[MAX_POSSIBLE_MOVES]);

#endif
