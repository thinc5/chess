#include "input.h"

#include <ctype.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include "board.h"

const char *COMMAND_STRINGS[] = {
    "COMMAND_INVALID",
    "COMMAND_HELP",
    "COMMAND_SAVE",
    "COMMAND_ANSWER",
    "COMMAND_LOAD",
    "COMMAND_FORFEIT",
    "COMMAND_CLEAR",
    "COMMAND_SELECT",
    "COMMAND_MOVE",
    "COMMAND_PROMOTION",
    "NUM_COMMANDS",
};

int
input_to_index(char x, char y)
{
    return ((y - '1') * BOARD_SIZE) + tolower(x) - 'a';
}

void
read_line(char *input_buffer, size_t *input_pointer)
{
    char c;
    while (true)
        {
            c = fgetc(stdin);
            if (c == EOF || c == '\n')
                break;
            else if (c == '\b' && *input_pointer > 0)
                {
                    input_buffer[*input_pointer] = '\0';
                    (*input_pointer)--;
                }
            else if (c < ' ' || c >= '~')
                {
                    continue;
                }
            else
                {
                    input_buffer[*input_pointer] = c;
                    (*input_pointer)++;
                }
        }
    input_buffer[*input_pointer] = '\0';
}

static inline bool
valid_char_pairing(char x, char y)
{
    return !(tolower(x) < 'a' || tolower(x) > 'a' + BOARD_SIZE || y < '0'
             || y > '0' + BOARD_SIZE);
}

/**
 * Pure textual validation with operation mode context, no real validation is
 * performed here. Returns intended command or COMMAND_INVALID if the command
 * is not matched.
 */
Command
parse_input(char input_buffer[INPUT_BUFFER_SIZE], OperationMode mode)
{
    // Global commands, always allowed.
    if (strncmp(input_buffer, "quit", INPUT_BUFFER_SIZE) == 0
        || strncmp(input_buffer, "exit", 4) == 0
        || strncmp(input_buffer, "ff", 4) == 0)
        {
            return COMMAND_FORFEIT;
        }

    if (strncmp(input_buffer, "?", INPUT_BUFFER_SIZE) == 0
        || strcmp(input_buffer, "help") == 0)
        {
            return COMMAND_HELP;
        }

    if (strncmp(input_buffer, "save", INPUT_BUFFER_SIZE) == 0)
        {
            return COMMAND_SAVE;
        }

    if (strncmp(input_buffer, "load", INPUT_BUFFER_SIZE) == 0)
        {
            return COMMAND_LOAD;
        }

    // Some commands are only avaliable in specific operation modes.
    switch (mode)
        {
        case OPERATION_SELECT:
            if (strnlen(input_buffer, INPUT_BUFFER_SIZE) == 2
                && valid_char_pairing(input_buffer[0], input_buffer[1]))
                {
                    return COMMAND_SELECT;
                }
            break;
        case OPERATION_MOVE:
            // Can only clear if we are in move mode.
            if (strncmp(input_buffer, "clear", INPUT_BUFFER_SIZE) == 0)
                {
                    return COMMAND_CLEAR;
                }
            if (strnlen(input_buffer, INPUT_BUFFER_SIZE) == 2
                && valid_char_pairing(input_buffer[0], input_buffer[1]))
                {
                    return COMMAND_MOVE;
                }
            break;
        case OPERATION_QUESTION:
            if (strnlen(input_buffer, INPUT_BUFFER_SIZE) == 1)
                {
                    switch (tolower(input_buffer[0]))
                        {
                        case 'y':
                        case 'n':
                            return COMMAND_ANSWER;
                        default:
                            break;
                        }
                }
            break;
        case OPERATION_PROMOTION:
            if (strnlen(input_buffer, INPUT_BUFFER_SIZE) == 1)
                {
                    // Valid piece?
                    switch (tolower(input_buffer[0]))
                        {
                        case 'q':
                        case 'k':
                        case 'r':
                        case 'b':
                            return COMMAND_PROMOTION;
                        default:
                            break;
                        }
                }
            break;
        default:
            break;
        }
    return COMMAND_INVALID;
}
