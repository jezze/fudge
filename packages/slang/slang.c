#include <fudge.h>
#include "token.h"

struct command
{

    struct token_string binary;
    struct token_string in;
    struct token_string din;
    struct token_string out;
    struct token_string dout;
    struct token_string data;

};

struct pipe
{

    struct command commands[8];
    unsigned int count;

};

struct expression
{

    struct pipe pipes[64];
    unsigned int count;

};

static unsigned int open_path(unsigned int index, struct token_string *string)
{

    return memory_match(string->position, "/", 1) ? call_open(index, CALL_DR, string->count - 1, string->position + 1) : call_open(index, CALL_DW, string->count, string->position);

}

static unsigned int open_pipe(unsigned int index, unsigned int number)
{

    char path[FUDGE_BSIZE];
    unsigned int offset = 0;

    offset += memory_write(path, FUDGE_BSIZE, "temp/", 5, offset);
    offset += memory_write_number(path, FUDGE_BSIZE, number, 10, offset);

    return call_open(index, CALL_DR, offset, path);

}

static unsigned int parse_path(struct token_state *state, struct token_string *string)
{

    unsigned int index = state->next;

    while (token_accept(state, TOKEN_TYPE_ALPHANUM | TOKEN_TYPE_DOT | TOKEN_TYPE_SLASH));

    string->position = state->buffer + index - 1;
    string->count = state->next - index;

    return string->count;

}

static unsigned int parse_in(struct token_state *state, struct token_string *string)
{

    if (!token_accept(state, TOKEN_TYPE_LT))
        return 0;

    while (token_accept(state, TOKEN_TYPE_SPACE));

    return parse_path(state, string);

}

static unsigned int parse_din(struct token_state *state, struct token_string *string)
{

    if (!token_accept(state, TOKEN_TYPE_LT))
        return 0;

    if (!token_accept(state, TOKEN_TYPE_LT))
        return 0;

    while (token_accept(state, TOKEN_TYPE_SPACE));

    return parse_path(state, string);

}

static unsigned int parse_out(struct token_state *state, struct token_string *string)
{

    if (!token_accept(state, TOKEN_TYPE_GT))
        return 0;

    while (token_accept(state, TOKEN_TYPE_SPACE));

    return parse_path(state, string);

}

static unsigned int parse_dout(struct token_state *state, struct token_string *string)
{

    if (!token_accept(state, TOKEN_TYPE_GT))
        return 0;

    if (!token_accept(state, TOKEN_TYPE_GT))
        return 0;

    while (token_accept(state, TOKEN_TYPE_SPACE));

    return parse_path(state, string);

}

static unsigned int parse_data(struct token_state *state, struct token_string *string)
{

    unsigned int index = state->next;

    if (!token_accept(state, TOKEN_TYPE_QUOTE))
        return 0;

    while (token_accept(state, ~TOKEN_TYPE_QUOTE));

    if (!token_accept(state, TOKEN_TYPE_QUOTE))
        return 0;

    string->position = state->buffer + index;
    string->count = state->next - index - 2;

    return string->count;

}

static unsigned int parse_command(struct token_state *state, struct command *command)
{

    if (!parse_path(state, &command->binary))
        return 0;

    while (token_accept(state, TOKEN_TYPE_SPACE));

    parse_in(state, &command->in);

    while (token_accept(state, TOKEN_TYPE_SPACE));

    parse_din(state, &command->din);

    while (token_accept(state, TOKEN_TYPE_SPACE));

    parse_out(state, &command->out);

    while (token_accept(state, TOKEN_TYPE_SPACE));

    parse_dout(state, &command->dout);

    while (token_accept(state, TOKEN_TYPE_SPACE));

    parse_data(state, &command->data);

    return 1;

}

static unsigned int parse_pipe(struct token_state *state, struct pipe *pipe)
{

    do
    {

        while (token_accept(state, TOKEN_TYPE_SPACE));

        if (!parse_command(state, &pipe->commands[pipe->count]))
            return 0;

        while (token_accept(state, TOKEN_TYPE_SPACE));

        pipe->count++;

    } while (token_accept(state, TOKEN_TYPE_PIPE));

    return 1;

}

static unsigned int parse(struct token_state *state, struct expression *expression)
{

    do
    {

        while (token_accept(state, TOKEN_TYPE_SPACE));

        if (!parse_pipe(state, &expression->pipes[expression->count]))
            return 0;

        while (token_accept(state, TOKEN_TYPE_SPACE));

        expression->count++;

    } while (token_accept(state, TOKEN_TYPE_SEMICOLON | TOKEN_TYPE_NEWLINE));

    return 1;

}

static void execute(struct expression *expression)
{

    unsigned int pindex;
    unsigned int cindex;

    call_open(CALL_D1, CALL_DI, 0, 0);
    call_open(CALL_D2, CALL_DO, 0, 0);
    call_open(CALL_D3, CALL_DW, 0, 0);

    for (pindex = 0; pindex < expression->count; pindex++)
    {

        struct pipe *pipe = &expression->pipes[pindex];

        for (cindex = 0; cindex < pipe->count; cindex++)
        {

            struct command *command = &pipe->commands[cindex];

            if (command->in.count)
                open_path(CALL_DI, &command->in);

            if (command->din.count)
                open_path(CALL_DC, &command->din);

            if (command->data.count)
                call_write(CALL_DI, 0, command->data.count, command->data.position);

            if (command->out.count)
                open_path(CALL_DO, &command->out);

            if (command->dout.count)
                open_path(CALL_DC, &command->dout);

            if (!command->out.count && pipe->count > 1 && cindex != pipe->count - 1)
                open_pipe(CALL_DO, cindex);

            call_open(CALL_DW, CALL_DR, 4, "bin/");
            open_path(CALL_D0, &command->binary);
            call_open(CALL_DW, CALL_D3, 0, 0);
            call_spawn(CALL_D0);
            call_open(CALL_DI, CALL_DO, 0, 0);
            call_open(CALL_DO, CALL_D2, 0, 0);

        }

        call_open(CALL_DI, CALL_D1, 0, 0);
        call_open(CALL_DO, CALL_D2, 0, 0);

    }

}

void main()
{

    char buffer[FUDGE_BSIZE];
    struct token_state state;
    struct expression expression;

    memory_clear(&expression, sizeof (struct expression));
    token_init_state(&state, call_read(CALL_DI, 0, FUDGE_BSIZE, buffer), buffer);
    parse(&state, &expression);
    execute(&expression);

}

