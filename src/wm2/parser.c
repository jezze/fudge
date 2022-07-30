#include <fudge.h>
#include <abi.h>
#include "widget.h"
#include "pool.h"
#include "parser.h"

struct state
{

    char *data;
    unsigned int count;
    unsigned int offset;
    unsigned int line;
    unsigned int linebreak;
    unsigned int quoted;
    unsigned int escaped;
    unsigned int errors;

};

char strdata[0x4000];
unsigned int strdataoffset;

static void fail(struct state *state)
{

    state->errors++;

}

static void addchar(struct state *state, char *result, unsigned int count, unsigned int i, char c)
{

    if (i < count)
        result[i] = c;
    else
        fail(state);

}

static unsigned int readword(struct state *state, char *result, unsigned int count)
{

    unsigned int i = 0;

    for (; state->offset < state->count; state->offset++)
    {

        char c = state->data[state->offset];

        switch (c)
        {

        case '\0':
        case '\n':
            state->escaped = 0;
            state->line++;
            state->linebreak = 1;
            state->offset++;

            addchar(state, result, count, i, '\0');

            return i;

        }

        if (state->escaped)
        {

            state->escaped = 0;

            switch (c)
            {

            case 'n':
                addchar(state, result, count, i, '\n');

                i++;

                break;

            default:
                addchar(state, result, count, i, c);

                i++;

                break;

            }

        }

        else
        {

            switch (c)
            {

            case '\\':
                state->escaped = 1;

                break;

            case ' ':
                if (state->quoted)
                {

                    addchar(state, result, count, i, c);

                    i++;

                }

                else
                {

                    addchar(state, result, count, i, '\0');

                    if (i == 0)
                        continue;

                    state->offset++;

                    return i;

                }

                break;

            case '"':
                state->quoted = !state->quoted;

                break;

            default:
                addchar(state, result, count, i, c);

                i++;

                break;

            }

        }

    }

    return 0;

}

static char *readunsafeword(struct state *state)
{

    char *word = strdata + strdataoffset;
    unsigned int count = readword(state, word, 0x4000 - strdataoffset);

    return (count) ? word : 0;

}

static char *readsafeword(struct state *state)
{

    char *word = strdata + strdataoffset;
    unsigned int count = readword(state, word, 0x4000 - strdataoffset);

    if (count)
    {

        strdataoffset += count + 1;

        return word;

    }

    return 0;

}

static unsigned int getcommand(struct state *state)
{

    char *word = readunsafeword(state);

    return (word) ? widget_getcommand(word) : 0;

}

static unsigned int gettype(struct state *state)
{

    char *word = readunsafeword(state);

    return (word) ? widget_gettype(word) : 0;

}

static void parseattributes(struct state *state, struct widget *widget)
{

    while (!state->errors && !state->linebreak)
    {

        char *word = readunsafeword(state);
        unsigned int attribute;

        if (!word)
            fail(state);

        attribute = widget_getattribute(word);
        word = readsafeword(state);

        if (!word)
            fail(state);

        widget_setattribute(widget, attribute, word);

    }

}

static void parsecomment(struct state *state)
{

    while (!state->errors && !state->linebreak)
        readunsafeword(state);

}

static void parsedelete(struct state *state)
{

    fail(state);

}

static void parseinsert(struct state *state, unsigned int source, char *in)
{

    unsigned int type = gettype(state);

    if (type)
    {

        struct widget *widget = pool_create(source, type, "", in);

        if (widget)
            parseattributes(state, widget);

    }

    else
    {

        fail(state);
        
    }

}

static void parseupdate(struct state *state, unsigned int source)
{

    fail(state);

}

static void parse(struct state *state, unsigned int source, char *in)
{

    while (!state->errors && state->offset < state->count)
    {

        state->linebreak = 0;

        switch (getcommand(state))
        {

        case WIDGET_COMMAND_NONE:
            break;

        case WIDGET_COMMAND_COMMENT:
            parsecomment(state);

            break;

        case WIDGET_COMMAND_DELETE:
            parsedelete(state);

            break;

        case WIDGET_COMMAND_INSERT:
            parseinsert(state, source, in);

            break;

        case WIDGET_COMMAND_UPDATE:
            parseupdate(state, source);

            break;

        default:
            fail(state);

            break;

        }

    }

}

void parser_parse(unsigned int source, char *in, unsigned int count, void *data)
{

    struct state state;

    state.data = data;
    state.count = count;
    state.offset = 0;
    state.line = 0;
    state.linebreak = 0;
    state.quoted = 0;
    state.escaped = 0;
    state.errors = 0;

    parse(&state, source, in);

}

