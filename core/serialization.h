#ifndef _SERIALIZATION_H
#define _SERIALIZATION_H

#include "game.h"

int serialize(ChessGame *game, char *filepath);
int deserialize(ChessGame *game, char *filepath);

#endif
