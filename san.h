#ifndef _SAN_H
#define _SAN_H

#include "pieces.h"

#include <stdbool.h>
#include <stdio.h>

typedef struct {
	size_t colour;
	EChessPiece piece;
	int origin[2];
	int destination[2];
	size_t capture;
	EChessPiece promotion;
	size_t check;
	size_t checkmate;
	size_t castle;
} SanData;

static const SanData EMPTY_SAN_DATA = {
	.colour = 0,
	.piece = 0,
	.origin = { -1, -1 },
	.destination = { -1, -1 },
	.capture = 0,
	.promotion = 0,
	.check = 0,
	.checkmate = 0
};

bool read_san_moves(FILE *file, SanData *data);

#endif
