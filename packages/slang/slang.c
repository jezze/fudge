#include <fudge.h>
#include "token.h"

struct string
{

    char *position;
    unsigned int index;
    unsigned int count;

};

struct input
{

    struct string path;
    struct string data;

};

struct output
{

    struct string path;

};

struct command
{

    struct string binary;
    struct input in0;
    struct input in1;
    struct output out0;

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

static unsigned int open_path(unsigned int index, struct string *string)
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

static unsigned int parse_path(struct token_state *state, struct string *string)
{

    string->index = state->current;

    while (token_accept(state, TOKEN_TYPE_ALPHANUM | TOKEN_TYPE_DOT | TOKEN_TYPE_SLASH));

    string->count = state->current - string->index;
    string->position = state->buffer + string->index;

    return string->count;

}

static unsigned int parse_data(struct token_state *state, struct string *string)
{

    if (!token_accept(state, TOKEN_TYPE_QUOTE))
        return 0;

    string->index = state->current;

    while (token_accept(state, ~TOKEN_TYPE_QUOTE));

    string->count = state->current - string->index;
    string->position = state->buffer + string->index;

    if (!token_accept(state, TOKEN_TYPE_QUOTE))
        return 0;

    return string->count;

}

static unsigned int parse_in0(struct token_state *state, struct input *input)
{

    if (!token_accept(state, TOKEN_TYPE_LT))
        return 0;

    token_skip(state, TOKEN_TYPE_SPACE);

    return parse_path(state, &input->path) || parse_data(state, &input->data);

}

static unsigned int parse_in1(struct token_state *state, struct input *input)
{

    if (!token_accept(state, TOKEN_TYPE_LT))
        return 0;

    if (!token_accept(state, TOKEN_TYPE_LT))
        return 0;

    token_skip(state, TOKEN_TYPE_SPACE);

    return parse_path(state, &input->path) || parse_data(state, &input->data);

}

static unsigned int parse_out(struct token_state *state, struct output *output)
{

    if (!token_accept(state, TOKEN_TYPE_GT))
        return 0;

    token_skip(state, TOKEN_TYPE_SPACE);

    return parse_path(state, &output->path);

}

static unsigned int parse_command(struct token_state *state, struct command *command)
{

    if (!parse_path(state, &command->binary))
        return 0;

    token_skip(state, TOKEN_TYPE_SPACE);
    parse_in0(state, &command->in0);
    token_skip(state, TOKEN_TYPE_SPACE);
    parse_in1(state, &command->in1);
    token_skip(state, TOKEN_TYPE_SPACE);
    parse_out(state, &command->out0);

    return 1;

}

static unsigned int parse_pipe(struct token_state *state, struct pipe *pipe)
{

    do
    {

        token_skip(state, TOKEN_TYPE_SPACE);

        if (!parse_command(state, &pipe->commands[pipe->count++]))
            return 0;

        token_skip(state, TOKEN_TYPE_SPACE);

    } while (token_accept(state, TOKEN_TYPE_PIPE));

    return 1;

}

static unsigned int parse(struct token_state *state, struct expression *expression)
{

    do
    {

        token_skip(state, TOKEN_TYPE_SPACE);

        if (!parse_pipe(state, &expression->pipes[expression->count++]))
            return 0;

        token_skip(state, TOKEN_TYPE_SPACE);

    } while (token_accept(state, TOKEN_TYPE_SEMICOLON | TOKEN_TYPE_NEWLINE));

    return 1;

}

static void execute(struct expression *expression)
{

    unsigned int pindex;
    unsigned int cindex;

    call_open(CALL_D1, CALL_DI, 0, 0);
    call_open(CALL_D2, CALL_DO, 0, 0);
    call_open(CALL_D3, CALL_DC, 0, 0);
    call_open(CALL_D4, CALL_DW, 0, 0);

    for (pindex = 0; pindex < expression->count; pindex++)
    {

        struct pipe *pipe = &expression->pipes[pindex];

        for (cindex = 0; cindex < pipe->count; cindex++)
        {

            struct command *command = &pipe->commands[cindex];

            if (command->in0.path.count)
                open_path(CALL_DI, &command->in0.path);

            if (command->in0.data.count)
                call_write(CALL_DI, 0, command->in0.data.count, command->in0.data.position);

            if (command->in1.path.count)
                open_path(CALL_DC, &command->in1.path);

            if (command->in1.data.count)
                call_write(CALL_DC, 0, command->in1.data.count, command->in1.data.position);

            if (command->out0.path.count)
                open_path(CALL_DO, &command->out0.path);

            if (!command->out0.path.count && pipe->count > 1 && cindex != pipe->count - 1)
                open_pipe(CALL_DO, cindex);

            call_open(CALL_DW, CALL_DR, 4, "bin/");
            open_path(CALL_D0, &command->binary);
            call_open(CALL_DW, CALL_D4, 0, 0);
            call_spawn(CALL_D0);
            call_close(CALL_D0);
            call_open(CALL_DI, CALL_DO, 0, 0);
            call_open(CALL_DO, CALL_D2, 0, 0);

        }

        call_open(CALL_DI, CALL_D1, 0, 0);
        call_open(CALL_DO, CALL_D2, 0, 0);
        call_open(CALL_DC, CALL_D3, 0, 0);

    }

    call_close(CALL_D1);
    call_close(CALL_D2);
    call_close(CALL_D3);
    call_close(CALL_D4);

}

void main()
{

    char buffer[FUDGE_BSIZE];
    struct token_state state;
    struct expression expression;

    memory_clear(&expression, sizeof (struct expression));
    token_init_state(&state, call_read(CALL_DI, 0, FUDGE_BSIZE, buffer), buffer);

    if (parse(&state, &expression))
        execute(&expression);

}

