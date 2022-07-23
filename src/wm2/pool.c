#include <fudge.h>
#include <abi.h>
#include "widget.h"
#include "pool.h"

#define MAX_WIDGETS                     1024

union payloads
{

    struct widget_button button;
    struct widget_fill fill;
    struct widget_image image;
    struct widget_container container;
    struct widget_textbox textbox;
    struct widget_window window;

};

static struct list widgetlist;
static struct list bumplist;
static struct list_item widgetitems[MAX_WIDGETS];
static struct widget widgets[MAX_WIDGETS];
static union payloads payloads[MAX_WIDGETS];
static unsigned int nwidgets;

struct list_item *pool_next(struct list_item *current)
{

    return (current) ? current->next : widgetlist.head;

}

struct list_item *pool_nextin(struct list_item *current, char *in)
{

    while ((current = pool_next(current)))
    {

        struct widget *widget = current->data;

        if (cstring_match(widget->in, in))
            return current;

    }

    return 0;

}

struct widget *pool_getwidgetbyid(char *id)
{

    struct list_item *current = 0;

    while ((current = pool_next(current)))
    {

        struct widget *widget = current->data;

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
    struct list_item *current = 0;

    list_move(&bumplist, &widgetlist, item);

    while ((current = pool_nextin(current, widget->id)))
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

struct widget *pool_create(unsigned int type, char *id, char *in)
{

    if (nwidgets < MAX_WIDGETS)
    {

        struct widget *widget = &widgets[nwidgets];
        struct list_item *item = &widgetitems[nwidgets];

        widget_init(widget, type, id, in, &payloads[nwidgets]);
        list_inititem(item, widget);
        list_add(&widgetlist, item);

        nwidgets++;

        return widget;

    }

    return 0;

}

void pool_setup(void)
{

    list_init(&widgetlist);
    list_init(&bumplist);

}

