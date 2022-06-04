#include "serialization.h"

#include <stdio.h>
#include <string.h>

#include "game.h"

int
serialize(ChessGame *game, const char *filepath)
{
    FILE *target = fopen(filepath, "w");
    if (target == NULL) { return 0; }
    if (fwrite(game, sizeof(ChessGame), 1, target) != 1) { return 0; }
    fclose(target);
    return 1;
}

int
deserialize(ChessGame *game, const char *filepath)
{
    ChessGame local;
    FILE *target = fopen(filepath, "r");
    if (target == NULL) { return 0; }
    if (fread(&local, sizeof(ChessGame), 1, target) != 1) { return 0; }
    fclose(target);
    memcpy(game, &local, sizeof(ChessGame));
    return 1;
}
