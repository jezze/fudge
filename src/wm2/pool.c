#include <fudge.h>
#include <abi.h>
#include "widget.h"
#include "pool.h"

#define MAX_WIDGETS                     1024

union payloads
{

    struct widget_button button;
    struct widget_container container;
    struct widget_fill fill;
    struct widget_image image;
    struct widget_textbox textbox;
    struct widget_window window;

};

static struct list widgetlist;
static struct list bumplist;
static struct list_item widgetitems[MAX_WIDGETS];
static struct widget widgets[MAX_WIDGETS];
static union payloads payloads[MAX_WIDGETS];
static unsigned int nwidgets;
static char strdata[0x4000];
static unsigned int strdataoffset;

struct list_item *pool_next(struct list_item *current)
{

    return (current) ? current->next : widgetlist.head;

}

struct list_item *pool_nextin(struct list_item *current, struct widget *parent)
{

    if (cstring_length(parent->id) == 0)
        return 0;

    while ((current = pool_next(current)))
    {

        struct widget *widget = current->data;

        if (!parent->source || widget->source == parent->source)
        {

            if (cstring_match(widget->in, parent->id))
                return current;

        }

    }

    return 0;

}

struct widget *pool_getwidgetbyid(unsigned int source, char *id)
{

    struct list_item *current = 0;

    while ((current = pool_next(current)))
    {

        struct widget *widget = current->data;

        if (widget->source != source)
            continue;

        if (cstring_match(widget->id, id))
            return widget;

    }

    return 0;

}

static struct list_item *finditem(struct widget *widget)
{

    struct list_item *current = 0;

    while ((current = pool_next(current)))
    {

        if (current->data == widget)
            return current;

    }

    return 0;

}

static void bump(struct list_item *item)
{

    struct widget *widget = item->data;
    struct list_item *current = item->prev;

    list_move(&bumplist, &widgetlist, item);

    while ((current = pool_nextin(current, widget)))
    {

        struct list_item *prev = current->prev;

        bump(current);

        current = prev;

    }

}

void pool_bump(struct widget *widget)
{

    struct list_item *item = finditem(widget);

    if (item)
        bump(item);

    while (bumplist.head)
        list_move(&widgetlist, &bumplist, bumplist.head);

}

struct widget *pool_create(unsigned int source, unsigned int type, char *id, char *in)
{

    if (nwidgets < MAX_WIDGETS)
    {

        struct widget *widget = &widgets[nwidgets];
        struct list_item *item = &widgetitems[nwidgets];

        widget_init(widget, source, type, id, in, &payloads[nwidgets]);
        list_inititem(item, widget);
        list_add(&widgetlist, item);

        nwidgets++;

        return widget;

    }

    return 0;

}

char *pool_savestring(char *string)
{

    unsigned int offset = strdataoffset;
    unsigned int length = cstring_length(string);

    strdataoffset += buffer_write(strdata, 0x4000, string, length + 1, strdataoffset);

    return strdata + offset;

}

char *pool_freestring(char *string)
{

    return 0;

}

char *pool_replacestring(char *current, char *string)
{

    if (current)
        current = pool_freestring(current);

    if (string)
        current = pool_savestring(string);

    return current;

}

void pool_setup(void)
{

    list_init(&widgetlist);
    list_init(&bumplist);

}

