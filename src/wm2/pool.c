#include <fudge.h>
#include <abi.h>
#include "widget.h"
#include "pool.h"

static struct list widgetlist;
static struct list_item widgetitems[32];
static struct widget widgets[32];
static struct widget_button buttons[32];
static struct widget_image images[32];
static struct widget_layout layouts[32];
static struct widget_window windows[32];
static unsigned int nwidgets = 0;

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

    struct widget *widget = &widgets[nwidgets];
    struct list_item *item = &widgetitems[nwidgets];

    widget_init(widget, type, id, in, data);
    list_inititem(item, widget);
    list_add(&widgetlist, item);

    nwidgets++;

    return widget;

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
    widget_initlayout(&layouts[0], LAYOUT_TYPE_FLOAT);
    widget_initwindow(&windows[0], "Window 0");
    widget_initwindow(&windows[1], "Window 1");
    widget_initimage(&images[0], 0, 0);
    widget_initbutton(&buttons[0], "Button0");

    windows[1].focus = 1;

    create(WIDGET_TYPE_LAYOUT, "root", "", &layouts[0]);
    create(WIDGET_TYPE_WINDOW, "window0", "root", &windows[0]);
    create(WIDGET_TYPE_WINDOW, "window1", "root", &windows[1]);
    create(WIDGET_TYPE_IMAGE, "mouse", "root", &images[0]);
    create(WIDGET_TYPE_BUTTON, "button0", "window1", &buttons[0]);

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

