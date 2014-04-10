#include <fudge.h>
#include "token.h"
#include "parse.h"

static unsigned int match_target(struct token_state *state, struct rule *rules, unsigned int count, struct string *target)
{

    unsigned int i;

    for (i = 0; i < count; i++)
    {

        struct rule *rule = &rules[i];
        char *t1 = state->buffer + rule->target.index;
        char *t2 = state->buffer + target->index;

        if (rule->target.count != target->count)
            continue;

        if (memory_match(t1, t2, target->count))
            return 1;

    }

    return 0;

}

static void execute(struct token_state *state, struct expression *expression)
{

    unsigned int rindex;
    unsigned int woff = 0;

    for (rindex = 0; rindex < expression->rules; rindex++)
    {

        struct rule *rule = &expression->rule[rindex];
        unsigned int pindex;

        for (pindex = 0; pindex < rule->prereqs; pindex++)
        {

            if (!match_target(state, expression->rule, rindex, &rule->prereq[pindex]))
                return;

        }

    }

    call_open(CALL_O0);

    for (rindex = 0; rindex < expression->rules; rindex++)
    {

        struct string *target = &expression->rule[rindex].target;

        woff += call_write(CALL_O0, woff, target->count, state->buffer + target->index);
        woff += call_write(CALL_O0, woff, 1, "\n");

    }

    call_close(CALL_O0);

}

void main()
{

    char buffer[FUDGE_BSIZE];
    struct token_state state;
    struct expression expression;

    memory_clear(&expression, sizeof (struct expression));
    call_open(CALL_I0);
    token_init_state(&state, call_read(CALL_I0, 0, FUDGE_BSIZE, buffer), buffer);
    call_close(CALL_I0);

    if (parse(&state, &expression))
        execute(&state, &expression);

}

