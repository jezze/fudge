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
    struct widget_layout layout;
    struct widget_textbox textbox;
    struct widget_window window;

};

static struct list widgetlist;
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

static struct widget *create(unsigned int type, char *id, char *in, void *data)
{

    if (nwidgets < MAX_WIDGETS)
    {

        struct widget *widget = &widgets[nwidgets];
        struct list_item *item = &widgetitems[nwidgets];

        widget_init(widget, type, id, in, data);
        list_inititem(item, widget);
        list_add(&widgetlist, item);

        nwidgets++;

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

    struct list_item *current = 0;
    struct widget *widget = item->data;

    list_move(&widgetlist, &widgetlist, item);

    while ((current = pool_nextin(current, widget->id)))
        bump(current);

}

void pool_bump(struct widget *widget)
{

    struct list_item *item = finditem(widget);

    if (item)
        bump(item);

}

void pool_setup(void)
{

    struct widget *widget;

    list_init(&widgetlist);
    widget_initlayout(&payloads[0].layout, LAYOUT_TYPE_FLOAT);
    widget_initfill(&payloads[1].fill, 0xFF142434);
    widget_initwindow(&payloads[2].window, "Window 0", 0);
    widget_initwindow(&payloads[3].window, "Window 1", 1);
    widget_initimage(&payloads[4].image, 0, 0);
    widget_initbutton(&payloads[5].button, "Click Me");
    widget_inittextbox(&payloads[6].textbox, 30, "Hello World! How are we today?");
    create(WIDGET_TYPE_LAYOUT, "root", "", &payloads[0]);
    create(WIDGET_TYPE_FILL, "background", "root", &payloads[1]);
    create(WIDGET_TYPE_WINDOW, "window0", "root", &payloads[2]);
    create(WIDGET_TYPE_WINDOW, "window1", "root", &payloads[3]);
    create(WIDGET_TYPE_IMAGE, "mouse", "root", &payloads[4]);
    create(WIDGET_TYPE_BUTTON, "button0", "window1", &payloads[5]);
    create(WIDGET_TYPE_TEXTBOX, "textbox0", "window1", &payloads[6]);

    widget = pool_getwidgetbyid("window0");
    widget->position.x = 200;
    widget->position.y = 100;
    widget->size.w = 800;
    widget->size.h = 600;
    widget = pool_getwidgetbyid("window1");
    widget->position.x = 100;
    widget->position.y = 80;
    widget->size.w = 800;
    widget->size.h = 600;
    widget = pool_getwidgetbyid("button0");
    widget->position.x = 100;
    widget->position.y = 800;
    widget->size.w = 240;
    widget->size.h = 60;

}

