#include <fudge.h>
#include "token.h"

static enum token_type tokenize(char c)
{

    switch (c)
    {

        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':

            return TOKEN_TYPE_NUM;

        case 'a':
        case 'b':
        case 'c':
        case 'd':
        case 'e':
        case 'f':
        case 'g':
        case 'h':
        case 'i':
        case 'j':
        case 'k':
        case 'l':
        case 'm':
        case 'n':
        case 'o':
        case 'p':
        case 'q':
        case 'r':
        case 's':
        case 't':
        case 'u':
        case 'v':
        case 'w':
        case 'x':
        case 'y':
        case 'z':
        case 'A':
        case 'B':
        case 'C':
        case 'D':
        case 'E':
        case 'F':
        case 'G':
        case 'H':
        case 'I':
        case 'J':
        case 'K':
        case 'L':
        case 'M':
        case 'N':
        case 'O':
        case 'P':
        case 'Q':
        case 'R':
        case 'S':
        case 'T':
        case 'U':
        case 'V':
        case 'W':
        case 'X':
        case 'Y':
        case 'Z':

            return TOKEN_TYPE_ALPHA;

        case ' ':
        case '\t':

            return TOKEN_TYPE_SPACE;

        case '<':

            return TOKEN_TYPE_LT;

        case '>':

            return TOKEN_TYPE_GT;

        case '-':

            return TOKEN_TYPE_MINUS;

        case ';':

            return TOKEN_TYPE_SEMICOLON;

        case '.':

            return TOKEN_TYPE_DOT;

        case '/':

            return TOKEN_TYPE_SLASH;

        case '|':

            return TOKEN_TYPE_PIPE;

        case '"':

            return TOKEN_TYPE_QUOTE;

        case '\n':

            return TOKEN_TYPE_NEWLINE;

    }

    return TOKEN_TYPE_NULL;

}

unsigned int token_next(struct token_state *state)
{

    if (state->current < state->count - 1)
        return ++state->current;

    return 0;

}

unsigned int token_accept(struct token_state *state, enum token_type type)
{

    if (tokenize(state->buffer[state->current]) & type)
        return token_next(state);

    return 0;

}

void token_skip(struct token_state *state, enum token_type type)
{

    while (token_accept(state, type));

}

void token_init_state(struct token_state *state, unsigned int count, char *buffer)
{

    memory_clear(state, sizeof (struct token_state));

    state->buffer = buffer;
    state->count = count;

}

