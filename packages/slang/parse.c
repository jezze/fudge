#include <fudge.h>
#include "token.h"
#include "parse.h"

static unsigned int parse_path(struct token_state *state, struct string *string)
{

    string->index = state->current;

    token_skip(state, TOKEN_TYPE_ALPHANUM | TOKEN_TYPE_DOT | TOKEN_TYPE_SLASH | TOKEN_TYPE_COLON);

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

        if (!parse_command(state, &pipe->command[pipe->commands++]))
            return 0;

        token_skip(state, TOKEN_TYPE_SPACE);

    } while (token_accept(state, TOKEN_TYPE_PIPE));

    return 1;

}

unsigned int parse(struct token_state *state, struct expression *expression)
{

    do
    {

        token_skip(state, TOKEN_TYPE_SPACE);

        if (!parse_pipe(state, &expression->pipe[expression->pipes++]))
            return 0;

        token_skip(state, TOKEN_TYPE_SPACE);

    } while (token_accept(state, TOKEN_TYPE_SEMICOLON | TOKEN_TYPE_NEWLINE));

    return 1;

}

