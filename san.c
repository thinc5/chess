#include "san.h"
#include "pieces.h"
#include "players.h"
#include "board.h"
#include "log.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

// Qa6xb7# fxg1=Q+
#define MAX_SAN_MOVE_LEN 15

ChessPiece san_sym_to_piece(char sym)
{
	switch (sym) {
	case 'K':
		return PIECE_KING;
	case 'Q':
		return PIECE_QUEEN;
	case 'R':
		return PIECE_ROOK;
	case 'B':
		return PIECE_BISHOP;
	case 'N':
		return PIECE_KNIGHT;
	case 'P':
		return PIECE_PAWN;
	default:
		return PIECE_NONE;
	}
}

static inline bool is_valid_rank(char sym)
{
	return sym >= 'a' && sym <= 'h';
}

static inline bool is_valid_file(char sym)
{
	return sym >= '1' && sym <= '8';
}

bool read_san_moves(FILE *file, SanData *data)
{
	for (int i = 0; i < 2; i++) {
		char current;
		SanData *san = &data[i];
		san->colour = i;
		san->piece = PIECE_PAWN;

		while ((current = fgetc(file)) != EOF) {
			// Are we finished processing?
			if (current == ' ' || current == '\n')
				break;

			// First we check for castling.
			if (current == 'O') {
				// Are we king side or castle side?
				size_t o_count = 1;
				while ((current = fgetc(file)) != EOF) {
					// Only valid characters.
					if (current != ' ' && current != 'O' &&
					    current != '-') {
						return false;
					}
					if (current == ' ') {
						// Rewind file so we capture it one loop up.
						fseek(file, -1, SEEK_CUR);
						break;
					}
					if (current == 'O') {
						o_count++;
					}
				}
				san->piece = PIECE_KING;
				san->castle = true;
				san->origin[0] = 4;
				san->origin[1] = san->colour ==
						 COLOUR_WHITE ? 0 : BOARD_SIZE -
						 1;
				san->destination[1] = san->origin[1];

				if (o_count == 2) {
					// Kingside.
					san->destination[0] = san->origin[0] +
							      2;
				} else if (o_count == 3) {
					// Queenside.
					san->destination[0] = san->origin[0] -
							      2;
				} else {
					return false;
				}
			}

			// Are we seeing a piece?
			ChessPiece piece = san_sym_to_piece(current);
			if (piece != PIECE_NONE && san->piece == PIECE_PAWN) {
				san->piece = piece;
				continue;
			}

			// Is this a capture?
			if (current == 'x') {
				san->capture = true;
				continue;
			}

			// Original position.
			if (is_valid_rank(current)) {
				// Always given at least a destination row, if we get two the
				// second is the destination.
				if (san->destination[0] == -1) {
					san->destination[0] = current - 'a';
				} else {
					san->origin[0] = san->destination[0];
					san->destination[0] = current - 'a';
				}
				continue;
			}

			if (is_valid_file(current)) {
				if (san->destination[1] == -1) {
					san->destination[1] = current - '1';
				} else {
					san->origin[1] = san->destination[1];
					san->destination[1] = current - '1';
				}
				continue;
			}

			// Promotion?
			if (current == '=') {
				current = fgetc(file);
				ChessPiece piece = san_sym_to_piece(current);
				if (piece == PIECE_NONE) {
					// Throw error
				}
				san->promotion = piece;
				continue;
			}

			// Check or checkmate?
			if (current == '+') {
				san->check = true;
				continue;
			}
			if (current == '#') {
				san->checkmate = true;
				continue;
			}
		}
	}

	return true;
}
