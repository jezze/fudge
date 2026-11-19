#include <fudge.h>
#include <abi.h>
#include "util.h"
#include "text.h"
#include "attr.h"
#include "widget.h"
#include "strpool.h"
#include "pool.h"
#include "record.h"
#include "parser.h"

#define NONE    1
#define COMMENT 2
#define DELETE  3
#define INSERT  4
#define UPDATE  5
#define RECORD  6
#define UNSET   7

#define NUM_COMMANDS                    7
#define NAME_SIZE                       256
#define TEMPLATE_SIZE                   4096
#define NUM_WIDGETS                     14
#define NUM_ATTRIBUTES                  26

static struct util_token commands[NUM_COMMANDS] =
{
    {NONE, ""},
    {COMMENT, "#"},
    {DELETE, "-"},
    {INSERT, "+"},
    {UPDATE, "="},
    {RECORD, "@"},
    {UNSET, "@-"}
};

static struct util_token widgets[NUM_WIDGETS] =
{
    {WIDGET_TYPE_BOX, "box"},
    {WIDGET_TYPE_BUTTON, "button"},
    {WIDGET_TYPE_CYCLE, "cycle"},
    {WIDGET_TYPE_EACH, "each"},
    {WIDGET_TYPE_FILL, "fill"},
    {WIDGET_TYPE_ITEM, "item"},
    {WIDGET_TYPE_LAYOUT, "layout"},
    {WIDGET_TYPE_MEDIA, "media"},
    {WIDGET_TYPE_PANEL, "panel"},
    {WIDGET_TYPE_SELECT, "select"},
    {WIDGET_TYPE_SWATCH, "swatch"},
    {WIDGET_TYPE_TAB, "tab"},
    {WIDGET_TYPE_TEXT, "text"},
    {WIDGET_TYPE_WINDOW, "window"}
};

static struct util_token attributes[NUM_ATTRIBUTES] =
{
    {ATTR_CHECKED, "checked"},
    {ATTR_COLOR, "color"},
    {ATTR_DATA, "data"},
    {ATTR_DISPLAY, "display"},
    {ATTR_FLOW, "flow"},
    {ATTR_HALIGN, "halign"},
    {ATTR_HEIGHT, "height"},
    {ATTR_ID, "id"},
    {ATTR_IN, "in"},
    {ATTR_LABEL, "label"},
    {ATTR_MIMETYPE, "mimetype"},
    {ATTR_MODE, "mode"},
    {ATTR_ONCHANGE, "onchange"},
    {ATTR_ONCLICK, "onclick"},
    {ATTR_ONENTER, "onenter"},
    {ATTR_ONRESIZE, "onresize"},
    {ATTR_ONSELECT, "onselect"},
    {ATTR_OVERFLOW, "overflow"},
    {ATTR_PADDING, "padding"},
    {ATTR_SOURCE, "source"},
    {ATTR_SPACING, "spacing"},
    {ATTR_SPAN, "span"},
    {ATTR_VALIGN, "valign"},
    {ATTR_WEIGHT, "weight"},
    {ATTR_WIDTH, "width"},
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
    unsigned int instance;

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

static unsigned int endword(struct state *state, char *result, unsigned int count, unsigned int i)
{

    if (i >= count)
    {

        fail(state);

        i = utf8_fit(result, count, count - 1);

    }

    result[i] = '\0';

    return i;

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

            return endword(state, result, count, i);

        }

        else if (c == ' ')
        {

            if (i || quotes)
            {

                return endword(state, result, count, i);

            }

        }

        else
        {

            i = addchar(state, result, count, i, c);

        }

    }

    state->linebreak = 1;

    return endword(state, result, count, i);

}

static unsigned int getcommand(struct state *state)
{

    unsigned int count = readword(state, strbuffer, 4096);

    return (count) ? util_getkey(commands, NUM_COMMANDS, strbuffer) : NONE;

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

        parent->cursor = pool_gettextlength(parent);
        parent->followcursor = 1;

    }

}

static unsigned int issamelabel(struct widget *widget, unsigned int attribute, char *value)
{

    if (attribute != ATTR_LABEL || !value || !widget->attributes.label)
        return 0;

    return cstring_match(strpool_getstring(widget->attributes.label), value);

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
            char *value = (count) ? strbuffer : 0;

            if (issamelabel(widget, attribute, value))
                continue;

            if (state->instance && (attribute == ATTR_ID || attribute == ATTR_IN))
                continue;

            widget_setattribute(widget, attribute, value);

            widget->changed = 1;

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

static void saveraw(struct widget *widget, char *text, unsigned int count)
{

    static char raw[TEMPLATE_SIZE];

    if (!buffer_findbyte(text, count, '{') || count >= TEMPLATE_SIZE)
        return;

    buffer_write(raw, TEMPLATE_SIZE, text, count, 0);

    raw[count] = '\0';
    widget->raw = strpool_updatestring(widget->raw, raw);

}

static unsigned int substitute(char *out, unsigned int size, char *raw, struct dataentry *record)
{

    unsigned int count = 0;
    unsigned int i = 0;

    while (raw[i])
    {

        unsigned int length = (raw[i] == '{') ? buffer_findbyte(raw + i + 1, cstring_length(raw + i + 1), '}') : 0;

        if (length && raw[i + 1 + length] == '}')
        {

            char *value = record_getfield(record, raw + i + 1, length);
            unsigned int j;

            for (j = 0; value[j]; j++)
                count += (value[j] == '"') ? buffer_write(out, size, "\"\\\"\"", 4, count) : buffer_write(out, size, value + j, 1, count);

            i += length + 2;

        }

        else
        {

            count += buffer_write(out, size, raw + i, 1, count);
            i++;

        }

    }

    return count;

}

static void applyrecord(struct widget *widget, struct widget *template, struct dataentry *record)
{

    static char text[TEMPLATE_SIZE];

    widget_copyattributes(widget, template);

    if (template->raw)
    {

        struct state state;

        state.data = text;
        state.count = substitute(text, TEMPLATE_SIZE, strpool_getstring(template->raw), record);
        state.offset = 0;
        state.line = 0;
        state.linebreak = 0;
        state.quoted = 0;
        state.errors = 0;
        state.instance = 1;

        parseattributes(&state, widget);

    }

    widget->changed = 1;

}

static struct widget *clone(struct widget *template, struct widget *parent, struct dataentry *record)
{

    struct widget *widget = pool_create(template->source, template->type, "", "");
    struct list_item *current = 0;

    if (!widget)
        return 0;

    pool_attach(widget, parent);
    applyrecord(widget, template, record);

    while ((current = pool_nextin(current, template)))
        clone(current->data, widget, record);

    return widget;

}

static void refresh(struct widget *widget, struct widget *template, struct dataentry *record)
{

    struct list_item *current = 0;
    struct list_item *other = 0;

    applyrecord(widget, template, record);

    while ((current = pool_nextin(current, widget)) && (other = pool_nextin(other, template)))
        refresh(current->data, other->data, record);

}

static unsigned int isinstanceof(struct widget *widget, char *key)
{

    return widget->instance && !widget->destroyed && cstring_match(strpool_getstring(widget->instance), key);

}

static void fill(struct widget *each, struct dataentry *record)
{

    char *key = record_getkey(record);
    struct list_item *template = 0;
    struct list_item *instance = 0;

    each->rebind = 1;

    while ((template = pool_nextin(template, each)))
    {

        struct widget *root = template->data;

        if (!widget_istemplate(root))
            continue;

        while ((instance = pool_nextin(instance, each)) && !(isinstanceof(instance->data, key) && widget_isshown(instance->data)));

        if (!instance)
            return;

        refresh(instance->data, root, record);

    }

}

static struct list_item *nextcopy(struct widget *each, struct list_item *current)
{

    while ((current = pool_nextin(current, each)))
    {

        struct widget *widget = current->data;

        if (widget->instance)
            return current;

    }

    return 0;

}

static void bindgroup(struct widget *each, struct list_item **instance, struct dataentry *record, unsigned int *changed)
{

    char *key = record_getkey(record);
    struct list_item *template = 0;

    while ((template = pool_nextin(template, each)))
    {

        struct widget *root = template->data;
        struct list_item *next;
        struct widget *widget;

        if (!widget_istemplate(root))
            continue;

        next = nextcopy(each, *instance);

        if (!next)
        {

            widget = clone(root, each, record);

            if (!widget)
                return;

            widget->instance = strpool_updatestring(0, "");
            next = nextcopy(each, *instance);

        }

        *instance = next;
        widget = next->data;

        if (!isinstanceof(widget, key) || !widget_isshown(widget))
        {

            refresh(widget, root, record);

            widget->instance = strpool_updatestring(widget->instance, key);
            *changed = 1;

        }

        widget->selected = record_isselected(record);
        widget->anchor = record_isanchor(record);

    }

}

unsigned int parser_bind(struct widget *each, unsigned int first, unsigned int count)
{

    char *set = strpool_getstring(each->attributes.data);
    struct dataentry *record = record_at(each->source, set, first);
    struct list_item *instance = 0;
    unsigned int changed = each->rowfirst != first || each->rowcount != count;
    unsigned int i;

    for (i = 0; i < count && record; i++, record = record_next(record, each->source, set))
        bindgroup(each, &instance, record, &changed);

    while ((instance = pool_nextin(instance, each)))
    {

        struct widget *widget = instance->data;

        if (widget->instance && widget_isshown(widget))
        {

            widget->attributes.display = ATTR_DISPLAY_NONE;
            widget->changed = 1;
            changed = 1;

        }

    }

    each->rowfirst = first;
    each->rowcount = i;
    each->rebind = 0;

    return changed;

}

unsigned int parser_substitute(char *out, unsigned int size, char *text, struct dataentry *record)
{

    unsigned int count = substitute(out, size - 1, text, record);

    out[count] = '\0';

    return count;

}

static unsigned int iseachof(struct widget *widget, unsigned int source, char *set)
{

    return widget->type == WIDGET_TYPE_EACH && widget->source == source && !widget->destroyed && cstring_match(strpool_getstring(widget->attributes.data), set);

}

static void empty(unsigned int source, char *set)
{

    struct list_item *current = 0;

    while ((current = pool_nextsource(current, source)))
    {

        struct widget *widget = current->data;

        if (iseachof(widget, source, set))
            widget->rebind = 1;

    }

}

static void parserecord(struct state *state, unsigned int source)
{

    char set[NAME_SIZE];
    char key[NAME_SIZE];
    struct dataentry *record;
    struct list_item *current = 0;

    readword(state, set, NAME_SIZE);

    if (state->linebreak)
    {

        fail(state);

        return;

    }

    readword(state, key, NAME_SIZE);

    record = record_find(source, set, key);

    if (!record)
        record = record_create(source, set, key);

    if (!record)
    {

        fail(state);

        return;

    }

    while (!state->errors && !state->linebreak)
    {

        char name[NAME_SIZE];

        readword(state, name, NAME_SIZE);

        if (state->linebreak)
            fail(state);
        else
            record_setfield(record, name, (readword(state, strbuffer, 4096)) ? strbuffer : "");

    }

    while ((current = pool_nextsource(current, source)))
    {

        if (iseachof(current->data, source, set))
            fill(current->data, record);

    }

}

static void rewind(unsigned int source, char *set)
{

    struct list_item *current = 0;

    while ((current = pool_nextsource(current, source)))
    {

        struct widget *widget = current->data;

        if (iseachof(widget, source, set))
        {

            for (; widget; widget = widget->parent)
                widget->scroll = util_position(0, 0);

        }

    }

}

static void parseunset(struct state *state, unsigned int source)
{

    char set[NAME_SIZE];
    char key[NAME_SIZE];

    readword(state, set, NAME_SIZE);

    if (state->linebreak)
    {

        empty(source, set);
        record_remove(source, set, 0);
        rewind(source, set);

        return;

    }

    readword(state, key, NAME_SIZE);
    empty(source, set);
    record_remove(source, set, key);

    while (!state->linebreak)
        readword(state, strbuffer, 4096);

}

static void parseinsert(struct state *state, unsigned int source, char *in)
{

    unsigned int type = getwidget(state);

    if (type)
    {

        struct widget *widget = pool_create(source, type, "", in);

        if (widget)
        {

            unsigned int start = state->offset;

            parseattributes(state, widget);
            saveraw(widget, state->data + start, state->offset - start);

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

        case RECORD:
            parserecord(state, source);

            break;

        case UNSET:
            parseunset(state, source);

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
    state.instance = 0;

    parse(&state, source, in);
    pool_setorphans();

}

