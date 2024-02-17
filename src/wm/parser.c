#include <fudge.h>
#include <abi.h>
#include "util.h"
#include "text.h"
#include "attr.h"
#include "widget.h"
#include "pool.h"
#include "parser.h"

#define NONE    1
#define COMMENT 2
#define DELETE  3
#define INSERT  4
#define UPDATE  5

static struct util_token commands[5] =
{
    {NONE, ""},
    {COMMENT, "#"},
    {DELETE, "-"},
    {INSERT, "+"},
    {UPDATE, "="}
};

static struct util_token widgets[11] =
{
    {WIDGET_TYPE_BUTTON, "button"},
    {WIDGET_TYPE_CHOICE, "choice"},
    {WIDGET_TYPE_FILL, "fill"},
    {WIDGET_TYPE_IMAGE, "image"},
    {WIDGET_TYPE_LAYOUT, "layout"},
    {WIDGET_TYPE_LISTBOX, "listbox"},
    {WIDGET_TYPE_SELECT, "select"},
    {WIDGET_TYPE_TEXT, "text"},
    {WIDGET_TYPE_TEXTBOX, "textbox"},
    {WIDGET_TYPE_TEXTBUTTON, "textbutton"},
    {WIDGET_TYPE_WINDOW, "window"}
};

static struct util_token attributes[20] =
{
    {ATTR_COLOR, "color"},
    {ATTR_COLUMNS, "columns"},
    {ATTR_CONTENT, "content"},
    {ATTR_CURSOR, "cursor"},
    {ATTR_FLOW, "flow"},
    {ATTR_HALIGN, "halign"},
    {ATTR_ID, "id"},
    {ATTR_IN, "in"},
    {ATTR_LABEL, "label"},
    {ATTR_MIMETYPE, "mimetype"},
    {ATTR_MODE, "mode"},
    {ATTR_ONCLICK, "onclick"},
    {ATTR_OVERFLOW, "overflow"},
    {ATTR_PADDING, "padding"},
    {ATTR_SPAN, "span"},
    {ATTR_SOURCE, "source"},
    {ATTR_TITLE, "title"},
    {ATTR_VALIGN, "valign"},
    {ATTR_WEIGHT, "weight"},
    {ATTR_WRAP, "wrap"}
};

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

static char strbuffer[4096];

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

static unsigned int getcommand(struct state *state)
{

    unsigned int count = readword(state, strbuffer, 4096);

    return (count) ? util_getkey(commands, 5, strbuffer) : 0;

}

static unsigned int getattribute(struct state *state)
{

    unsigned int count = readword(state, strbuffer, 4096);

    return (count) ? util_getkey(attributes, 20, strbuffer) : 0;

}

static unsigned int getwidget(struct state *state)
{

    unsigned int count = readword(state, strbuffer, 4096);

    return (count) ? util_getkey(widgets, 11, strbuffer) : 0;

}

static void parseattributes(struct state *state, struct widget *widget)
{

    while (!state->errors && !state->linebreak)
    {

        unsigned int attribute = getattribute(state);

        if (attribute)
        {

            unsigned int count = readword(state, strbuffer, 4096);

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
        readword(state, strbuffer, 4096);

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

    unsigned int count = readword(state, strbuffer, 4096);

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

    unsigned int type = getwidget(state);

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

    unsigned int count = readword(state, strbuffer, 4096);

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

        case NONE:
            break;

        case COMMENT:
            parsecomment(state);

            break;

        case DELETE:
            parsedelete(state, source);

            break;

        case INSERT:
            parseinsert(state, source, in);

            break;

        case UPDATE:
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

