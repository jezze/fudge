#include <fudge.h>
#include <abi.h>
#include "util.h"
#include "text.h"
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

static char strbuffer[BUFFER_SIZE];

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

    state->linebreak = 0;
    state->escaped = 0;

    for (; state->offset < state->count; state->offset++)
    {

        char c = state->data[state->offset];

        if (state->escaped)
        {

            state->escaped = 0;

            switch (c)
            {

            case 'n':
                addchar(state, result, count, i, '\n');

                i++;

                break;

            case '"':
                addchar(state, result, count, i, '"');

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

            if (state->quoted)
            {

                switch (c)
                {

                case '"':
                    state->quoted = 0;

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

                case '"':
                    state->quoted = 1;

                    break;

                case '\0':
                case '\n':
                    state->line++;
                    state->linebreak = 1;
                    state->offset++;

                    addchar(state, result, count, i, '\0');

                    return i;

                case ' ':
                    addchar(state, result, count, i, '\0');

                    if (i == 0)
                        continue;

                    state->offset++;

                    return i;

                default:
                    addchar(state, result, count, i, c);

                    i++;

                    break;

                }

            }

        }

    }

    return 0;

}

static unsigned int getattribute(struct state *state)
{

    unsigned int count = readword(state, strbuffer, BUFFER_SIZE);

    return (count) ? widget_getattribute(strbuffer) : 0;

}

static unsigned int getcommand(struct state *state)
{

    unsigned int count = readword(state, strbuffer, BUFFER_SIZE);

    return (count) ? widget_getcommand(strbuffer) : 0;

}

static unsigned int gettype(struct state *state)
{

    unsigned int count = readword(state, strbuffer, BUFFER_SIZE);

    return (count) ? widget_gettype(strbuffer) : 0;

}

static void parseattributes(struct state *state, struct widget *widget)
{

    while (!state->errors && !state->linebreak)
    {

        unsigned int attribute = getattribute(state);

        if (attribute)
        {

            unsigned int count = readword(state, strbuffer, BUFFER_SIZE);

            if (count)
                widget_setattribute(widget, attribute, strbuffer);
            else
                widget_setattribute(widget, attribute, 0);

        }

        else
        {

            fail(state);

        }

    }

}

static void parsecomment(struct state *state)
{

    while (!state->errors && !state->linebreak)
        readword(state, strbuffer, BUFFER_SIZE);

}

static void deletechildren(struct widget *widget)
{

    struct list_item *current = 0;

    while ((current = pool_nextin(current, widget)))
    {

        struct widget *child = current->data;

        widget_setstate(child, WIDGET_STATE_DESTROYED);
        deletechildren(child);

    }

}

static void parsedelete(struct state *state, unsigned int source)
{

    unsigned int count = readword(state, strbuffer, BUFFER_SIZE);

    if (count)
    {

        struct widget *widget = pool_getwidgetbyid(source, strbuffer);

        if (widget)
        {

            widget_setstate(widget, WIDGET_STATE_DESTROYED);
            deletechildren(widget);

        }

        else
        {

            fail(state);

        }

    }

    else
    {

        fail(state);

    }

}

static void parseinsert(struct state *state, unsigned int source, char *in)
{

    unsigned int type = gettype(state);

    if (type)
    {

        struct widget *widget = pool_create(source, type, "", in);

        if (widget)
            parseattributes(state, widget);
        else
            fail(state);

    }

    else
    {

        fail(state);
        
    }

}

static void parseupdate(struct state *state, unsigned int source)
{

    unsigned int count = readword(state, strbuffer, BUFFER_SIZE);

    if (count)
    {

        struct widget *widget = pool_getwidgetbyid(source, strbuffer);

        if (widget)
            parseattributes(state, widget);
        else
            fail(state);

    }

    else
    {

        fail(state);

    }

}

static void parse(struct state *state, unsigned int source, char *in)
{

    while (!state->errors && state->offset < state->count)
    {

        switch (getcommand(state))
        {

        case WIDGET_COMMAND_NONE:
            break;

        case WIDGET_COMMAND_COMMENT:
            parsecomment(state);

            break;

        case WIDGET_COMMAND_DELETE:
            parsedelete(state, source);

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

