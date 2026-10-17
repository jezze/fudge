#include <fudge.h>
#include <abi.h>
#include <compression.h>
#include <image.h>
#include "util.h"
#include "text.h"
#include "attr.h"
#include "widget.h"
#include "strpool.h"
#include "pool.h"
#include "parser.h"

#define NONE    1
#define COMMENT 2
#define DELETE  3
#define INSERT  4
#define UPDATE  5

#define NUM_COMMANDS                    5
#define NUM_WIDGETS                     12
#define NUM_ATTRIBUTES                  23

static struct util_token commands[NUM_COMMANDS] =
{
    {NONE, ""},
    {COMMENT, "#"},
    {DELETE, "-"},
    {INSERT, "+"},
    {UPDATE, "="}
};

static struct util_token widgets[NUM_WIDGETS] =
{
    {WIDGET_TYPE_BOX, "box"},
    {WIDGET_TYPE_BUTTON, "button"},
    {WIDGET_TYPE_CHECKBOX, "checkbox"},
    {WIDGET_TYPE_FILL, "fill"},
    {WIDGET_TYPE_IMAGE, "image"},
    {WIDGET_TYPE_ITEM, "item"},
    {WIDGET_TYPE_LAYOUT, "layout"},
    {WIDGET_TYPE_PANEL, "panel"},
    {WIDGET_TYPE_SELECT, "select"},
    {WIDGET_TYPE_SWATCH, "swatch"},
    {WIDGET_TYPE_TEXT, "text"},
    {WIDGET_TYPE_WINDOW, "window"}
};

static struct util_token attributes[NUM_ATTRIBUTES] =
{
    {ATTR_CHECKED, "checked"},
    {ATTR_COLOR, "color"},
    {ATTR_CURSOR, "cursor"},
    {ATTR_DISPLAY, "display"},
    {ATTR_FLOW, "flow"},
    {ATTR_HALIGN, "halign"},
    {ATTR_ID, "id"},
    {ATTR_IN, "in"},
    {ATTR_LABEL, "label"},
    {ATTR_MIMETYPE, "mimetype"},
    {ATTR_MODE, "mode"},
    {ATTR_ONCHANGE, "onchange"},
    {ATTR_ONCLICK, "onclick"},
    {ATTR_ONENTER, "onenter"},
    {ATTR_ONSELECT, "onselect"},
    {ATTR_OVERFLOW, "overflow"},
    {ATTR_PADDING, "padding"},
    {ATTR_SOURCE, "source"},
    {ATTR_SPACING, "spacing"},
    {ATTR_SPAN, "span"},
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
    unsigned int errors;

};

static char strbuffer[4096];

static void fail(struct state *state)
{

    state->errors++;

}

static unsigned int addchar(struct state *state, char *result, unsigned int count, unsigned int i, char c)
{

    if (i < count)
        result[i] = c;
    else
        fail(state);

    return i + 1;

}

static unsigned int readword(struct state *state, char *result, unsigned int count)
{

    unsigned int i = 0;
    unsigned int quotes = 0;

    state->linebreak = 0;

    while (state->offset < state->count)
    {

        char c = state->data[state->offset++];

        if (state->quoted)
        {

            if (c == '"')
                state->quoted = 0;
            else
                i = addchar(state, result, count, i, c);

        }

        else if (c == '\\' && state->offset < state->count)
        {

            c = state->data[state->offset++];
            i = addchar(state, result, count, i, (c == 'n') ? '\n' : c);

        }

        else if (c == '"')
        {

            state->quoted = 1;
            quotes = 1;

        }

        else if (c == '\n' || c == '\0')
        {

            state->line++;
            state->linebreak = 1;

            addchar(state, result, count, i, '\0');

            return i;

        }

        else if (c == ' ')
        {

            if (i || quotes)
            {

                addchar(state, result, count, i, '\0');

                return i;

            }

        }

        else
        {

            i = addchar(state, result, count, i, c);

        }

    }

    state->linebreak = 1;

    addchar(state, result, count, i, '\0');

    return i;

}

static unsigned int getcommand(struct state *state)
{

    unsigned int count = readword(state, strbuffer, 4096);

    return (count) ? util_getkey(commands, NUM_COMMANDS, strbuffer) : 0;

}

static unsigned int getattribute(struct state *state)
{

    unsigned int count = readword(state, strbuffer, 4096);

    return (count) ? util_getkey(attributes, NUM_ATTRIBUTES, strbuffer) : 0;

}

static unsigned int getwidget(struct state *state)
{

    unsigned int count = readword(state, strbuffer, 4096);

    return (count) ? util_getkey(widgets, NUM_WIDGETS, strbuffer) : 0;

}

static void movecursortoend(struct widget *widget)
{

    struct widget *parent = widget->parent;

    if (parent && widget_iseditable(parent))
    {

        parent->attributes.cursor = pool_gettextlength(parent);
        parent->followcursor = 1;

    }

}

static void parseattributes(struct state *state, struct widget *widget)
{

    unsigned int labelset = 0;

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

            if (attribute == ATTR_IN)
                pool_setparent(widget);

            if (attribute == ATTR_LABEL)
                labelset = 1;

        }

        else
        {

            fail(state);

        }

    }

    if (labelset && widget->type == WIDGET_TYPE_TEXT)
        movecursortoend(widget);

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

        child->destroyed = 1;
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

            widget->destroyed = 1;
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
    state.errors = 0;

    parse(&state, source, in);
    pool_setorphans();

}

