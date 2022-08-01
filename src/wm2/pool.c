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
    struct widget_grid grid;
    struct widget_image image;
    struct widget_text text;
    struct widget_textbox textbox;
    struct widget_window window;

};

struct strindex
{

    unsigned int offset;
    unsigned int length;

};

static struct list widgetlist;
static struct list bumplist;
static struct list_item widgetitems[MAX_WIDGETS];
static struct widget widgets[MAX_WIDGETS];
static union payloads payloads[MAX_WIDGETS];
static unsigned int nwidgets;
static char strdata[0x4000];
static unsigned int strdataoffset;
static struct strindex strindex[512];

struct list_item *pool_next(struct list_item *current)
{

    return (current) ? current->next : widgetlist.head;

}

struct list_item *pool_nextin(struct list_item *current, struct widget *parent)
{

    if (!pool_getcstringlength(parent->id))
        return 0;

    while ((current = pool_next(current)))
    {

        struct widget *widget = current->data;

        if (!parent->source || widget->source == parent->source)
        {

            if (cstring_match(pool_getstring(widget->in), pool_getstring(parent->id)))
                return current;

        }

    }

    return 0;

}

struct list_item *pool_nextsource(struct list_item *current, unsigned int source)
{

    while ((current = pool_next(current)))
    {

        struct widget *widget = current->data;

        if (widget->source == source)
            return current;

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

        if (cstring_match(pool_getstring(widget->id), id))
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

static unsigned int findslot(void)
{

    unsigned int i;

    for (i = 1; i < 512; i++)
    {

        if (strindex[i].offset == 0 && strindex[i].length == 0)
            return i;

    }

    return 0;

}

char *pool_getstring(unsigned int index)
{

    return strdata + strindex[index].offset;

}

unsigned int pool_getcstringlength(unsigned int index)
{

    return (strindex[index].length) ? strindex[index].length - 1 : 0;

}

unsigned int pool_getcstringlengthz(unsigned int index)
{

    return (strindex[index].length) ? strindex[index].length : 0;

}

unsigned int pool_savedata(unsigned int count, void *data)
{

    unsigned int slot = findslot();

    if (slot)
    {

        struct strindex *index = &strindex[slot];

        index->offset = strdataoffset;
        index->length = count;
        strdataoffset += buffer_write(strdata, 0x4000, data, index->length, index->offset);

    }

    return slot;

}

unsigned int pool_savecstring(char *cstring)
{

    return pool_savedata(cstring_lengthz(cstring), cstring);

}

unsigned int pool_freedata(unsigned int index)
{

    struct strindex *s = &strindex[index];
    unsigned int offset = s->offset;
    unsigned int length = s->length;
    unsigned int next = s->offset + s->length;
    unsigned int count = strdataoffset - next;
    unsigned int i;

    buffer_write(strdata, 0x4000, strdata + next, count, offset);
    strdataoffset -= length;

    s->offset = 0;
    s->length = 0;

    for (i = 0; i < 512; i++)
    {

        struct strindex *current = &strindex[i];

        if (current->offset > offset)
            current->offset -= length;

    }

    return 0;

}

unsigned int pool_replacecstring(unsigned int index, char *cstring)
{

    if (index)
        index = pool_freedata(index);

    if (cstring)
        index = pool_savecstring(cstring);

    return index;

}

void pool_setup(void)
{

    list_init(&widgetlist);
    list_init(&bumplist);

}

