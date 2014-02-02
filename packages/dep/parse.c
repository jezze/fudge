#include <fudge.h>
#include "token.h"
#include "parse.h"

static unsigned int parse_path(struct token_state *state, struct string *string)
{

    string->index = state->current;

    token_skip(state, TOKEN_TYPE_PATH);

    string->count = state->current - string->index;

    return string->count;

}

static unsigned int parse_rule(struct token_state *state, struct rule *rule)
{

    if (!parse_path(state, &rule->target))
        return 0;

    if (!token_accept(state, TOKEN_TYPE_COLON))
        return 0;

    if (!token_accept(state, TOKEN_TYPE_SPACE))
        return 1;

    do
    {

        parse_path(state, &rule->prereq[rule->prereqs++]);

    } while (token_accept(state, TOKEN_TYPE_SPACE));

    return 1;

}

unsigned int parse(struct token_state *state, struct expression *expression)
{

    do
    {

        if (!parse_rule(state, &expression->rule[expression->rules++]))
            return 0;

    } while (token_accept(state, TOKEN_TYPE_NEWLINE));

    return 1;

}

