#ifndef _SERIALIZATION_H
#define _SERIALIZATION_H

#include "game.h"

int serialize(ChessGame *game, const char *filepath);
int deserialize(ChessGame *game, const char *filepath);

#endif
