#ifndef _INPUT_H
#define _INPUT_H

#include <stddef.h>

#define INPUT_BUFFER_SIZE 1024

typedef enum {
	OPERATION_SELECT,
	OPERATION_MOVE,
	OPERATION_QUESTION,
	OPERATION_PROMOTION,
	NUM_OPERATION_MODES
} OperationMode;

typedef enum {
	COMMAND_INVALID,
	COMMAND_HELP,
	COMMAND_SAVE,
	COMMAND_ANSWER,
	COMMAND_LOAD,
	COMMAND_FORFEIT,
	COMMAND_CLEAR,
	COMMAND_SELECT,
	COMMAND_MOVE,
	COMMAND_PROMOTION,
	NUM_COMMANDS,
} Command;

extern const char *COMMAND_STRINGS[];

int input_to_index(char x, char y);
void read_line(char *input_buffer, size_t *input_pointer);
Command parse_input(char input_buffer[INPUT_BUFFER_SIZE], OperationMode mode);

#endif
