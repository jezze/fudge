#include <fudge.h>
#include "token.h"
#include "parse.h"

static unsigned int parse_num(struct token_state *state, struct string *string)
{

    string->index = state->current;

    token_skip(state, TOKEN_TYPE_NUM);

    string->count = state->current - string->index;

    return string->count;

}

static unsigned int parse_path(struct token_state *state, struct string *string)
{

    string->index = state->current;

    token_skip(state, TOKEN_TYPE_ALPHANUM | TOKEN_TYPE_DOT | TOKEN_TYPE_SLASH);

    string->count = state->current - string->index;

    return string->count;

}

static unsigned int parse_data(struct token_state *state, struct string *string)
{

    if (!token_accept(state, TOKEN_TYPE_QUOTE))
        return 0;

    string->index = state->current;

    token_skip(state, ~TOKEN_TYPE_QUOTE);

    string->count = state->current - string->index;

    if (!token_accept(state, TOKEN_TYPE_QUOTE))
        return 0;

    return string->count;

}

static unsigned int parse_in(struct token_state *state, struct input *input)
{

    if (!token_accept(state, TOKEN_TYPE_LT))
        return 0;

    parse_num(state, &input->index);
    token_skip(state, TOKEN_TYPE_SPACE);

    return parse_path(state, &input->path) || parse_data(state, &input->data);

}

static unsigned int parse_out(struct token_state *state, struct output *output)
{

    if (!token_accept(state, TOKEN_TYPE_GT))
        return 0;

    parse_num(state, &output->index);
    token_skip(state, TOKEN_TYPE_SPACE);

    return parse_path(state, &output->path);

}

static unsigned int parse_command(struct token_state *state, struct command *command)
{

    if (!parse_path(state, &command->binary))
        return 0;

    token_skip(state, TOKEN_TYPE_SPACE);

    while (parse_in(state, &command->in[command->ins]))
    {

        token_skip(state, TOKEN_TYPE_SPACE);
        command->ins++;

    }

    while (parse_out(state, &command->out[command->outs]))
    {

        token_skip(state, TOKEN_TYPE_SPACE);
        command->outs++;

    }

    return 1;

}

static unsigned int parse_pipe(struct token_state *state, struct pipe *pipe)
{

    do
    {

        token_skip(state, TOKEN_TYPE_SPACE);

        if (parse_command(state, &pipe->command[pipe->commands]))
            pipe->commands++;

        token_skip(state, TOKEN_TYPE_SPACE);

    } while (token_accept(state, TOKEN_TYPE_PIPE));

    return pipe->commands;

}

unsigned int parse(struct token_state *state, struct expression *expression)
{

    do
    {

        token_skip(state, TOKEN_TYPE_SPACE);

        if (parse_pipe(state, &expression->pipe[expression->pipes]))
            expression->pipes++;

        token_skip(state, TOKEN_TYPE_SPACE);

    } while (token_accept(state, TOKEN_TYPE_SEMICOLON | TOKEN_TYPE_NEWLINE));

    return expression->pipes;

}

