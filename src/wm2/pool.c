#include <fudge.h>
#include <abi.h>
#include "widget.h"
#include "pool.h"

static struct list widgetlist;
static struct list_item widgetitems[32];
static struct widget widgets[32];
static struct window windows[32];
static struct layout layouts[32];

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

void pool_setup(void)
{

    struct widget *widget;
    struct list_item *widgetitem;
    unsigned int nwidgets = 0;

    list_init(&widgetlist);

    /* Root */
    widget = &widgets[nwidgets];
    widgetitem = &widgetitems[nwidgets];
    nwidgets++;

    widget_init(widget, WIDGET_TYPE_LAYOUT, "root", "", &layouts[0]);
    layout_init(&layouts[0], LAYOUT_TYPE_FLOAT);
    list_inititem(widgetitem, widget);
    list_add(&widgetlist, widgetitem);

    /* Window0 */
    widget = &widgets[nwidgets];
    widgetitem = &widgetitems[nwidgets];
    nwidgets++;

    widget_init(widget, WIDGET_TYPE_WINDOW, "window0", "root", &windows[0]);
    window_init(&windows[0], "Window 0", 200, 100, 800, 600);
    list_inititem(widgetitem, widget);
    list_add(&widgetlist, widgetitem);

    /* Window1 */
    widget = &widgets[nwidgets];
    widgetitem = &widgetitems[nwidgets];
    nwidgets++;

    widget_init(widget, WIDGET_TYPE_WINDOW, "window1", "root", &windows[1]);
    window_init(&windows[1], "Window 1", 100, 80, 800, 600);
    list_inititem(widgetitem, widget);
    list_add(&widgetlist, widgetitem);

    windows[1].focus = 1;

}

