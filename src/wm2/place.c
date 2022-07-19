#include <fudge.h>
#include <abi.h>
#include "widget.h"
#include "pool.h"
#include "place.h"

static void placebutton(struct widget *widget, int x, int y, unsigned int w, unsigned int h)
{

    widget->position.x = x;
    widget->position.y = y;
    widget->size.w = 200;
    widget->size.h = 80;

}

static void placefill(struct widget *widget, int x, int y, unsigned int w, unsigned int h)
{

    widget->position.x = x;
    widget->position.y = y;
    widget->size.w = w;
    widget->size.h = h;

}

static void placeimage(struct widget *widget, int x, int y, unsigned int w, unsigned int h)
{

}

static void placelayout(struct widget *widget, int x, int y, unsigned int w, unsigned int h)
{

    struct widget_layout *layout = widget->data;
    struct list_item *current = 0;

    widget->position.x = x;
    widget->position.y = y;
    widget->size.w = w;
    widget->size.h = h;

    switch (layout->type)
    {

    case LAYOUT_TYPE_VERTICAL:
        break;

    case LAYOUT_TYPE_FLOAT:
        while ((current = pool_nextin(current, widget->id)))
        {

            struct widget *child = current->data;

            place_widget(child, widget->position.x, widget->position.y, widget->size.w, widget->size.h);

        }

        break;

    }

}

static void placewindow(struct widget *widget, int x, int y, unsigned int w, unsigned int h)
{

    struct list_item *current = 0;

    while ((current = pool_nextin(current, widget->id)))
    {

        struct widget *child = current->data;

        place_widget(child, widget->position.x + 16, widget->position.y + 16 + 38, widget->size.w - 24, widget->size.h - 24);

    }

}

void place_widget(struct widget *widget, int x, int y, unsigned int w, unsigned int h)
{

    switch (widget->type)
    {

    case WIDGET_TYPE_BUTTON:
        placebutton(widget, x, y, w, h);

        break;

    case WIDGET_TYPE_FILL:
        placefill(widget, x, y, w, h);

        break;

    case WIDGET_TYPE_IMAGE:
        placeimage(widget, x, y, w, h);

        break;

    case WIDGET_TYPE_LAYOUT:
        placelayout(widget, x, y, w, h);

        break;

    case WIDGET_TYPE_WINDOW:
        placewindow(widget, x, y, w, h);

        break;

    }

}

