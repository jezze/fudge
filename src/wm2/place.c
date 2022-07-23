#include <fudge.h>
#include <abi.h>
#include "util.h"
#include "widget.h"
#include "pool.h"
#include "place.h"

#define WINDOWPADDING                   16

static void placebutton(struct widget *widget, int x, int y, unsigned int w, unsigned int h)
{

    widget->position.x = x;
    widget->position.y = y;
    widget->size.w = 200;
    widget->size.h = 80;

}

static void placecontainerfloat(struct widget *widget, int x, int y, unsigned int w, unsigned int h)
{

    struct list_item *current = 0;

    while ((current = pool_nextin(current, widget->id)))
    {

        struct widget *child = current->data;

        place_widget(child, x, y, w, h);

    }

    widget->position.x = x;
    widget->position.y = y;
    widget->size.w = w;
    widget->size.h = h;

}

static void placecontainerhorizontal(struct widget *widget, int x, int y, unsigned int w, unsigned int h)
{

    struct list_item *current = 0;
    int cw = 0;

    while ((current = pool_nextin(current, widget->id)))
    {

        struct widget *child = current->data;

        place_widget(child, x + cw, y, 0, h);
        child->size.h = util_clamp(child->size.h, 0, h);
        child->size.w = util_clamp(child->size.w, 0, w - cw);

        cw += child->size.w;

    }

    widget->position.x = x;
    widget->position.y = y;
    widget->size.w = util_clamp(cw, 0, w);
    widget->size.h = h;

}

static void placecontainervertical(struct widget *widget, int x, int y, unsigned int w, unsigned int h)
{

    struct list_item *current = 0;
    int ch = 0;

    while ((current = pool_nextin(current, widget->id)))
    {

        struct widget *child = current->data;

        place_widget(child, x, y + ch, w, 0);
        child->size.h = util_clamp(child->size.h, 0, h - ch);
        child->size.w = util_clamp(child->size.w, 0, w);

        ch += child->size.h;

    }

    widget->position.x = x;
    widget->position.y = y;
    widget->size.w = w;
    widget->size.h = util_clamp(ch, 0, h);

}

static void placecontainer(struct widget *widget, int x, int y, unsigned int w, unsigned int h)
{

    struct widget_container *container = widget->data;

    switch (container->layout)
    {

    case CONTAINER_LAYOUT_FLOAT:
        placecontainerfloat(widget, x, y, w, h);
        
        break;

    case CONTAINER_LAYOUT_HORIZONTAL:
        placecontainerhorizontal(widget, x, y, w, h);

        break;

    case CONTAINER_LAYOUT_VERTICAL:
        placecontainervertical(widget, x, y, w, h);

        break;

    case CONTAINER_LAYOUT_GRID:
        widget->position.x = x;
        widget->position.y = y;
        widget->size.w = 0;
        widget->size.h = 0;

        break;

    }

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

static void placetextbox(struct widget *widget, int x, int y, unsigned int w, unsigned int h)
{

    widget->position.x = x;
    widget->position.y = y;
    widget->size.w = w;
    widget->size.h = 32; /* dynamically change depending on text */

}

static void placewindow(struct widget *widget, int x, int y, unsigned int w, unsigned int h)
{

    struct list_item *current = 0;

    while ((current = pool_nextin(current, widget->id)))
    {

        struct widget *child = current->data;

        place_widget(child, widget->position.x + WINDOWPADDING, widget->position.y + WINDOWPADDING + 40, widget->size.w - WINDOWPADDING * 2, widget->size.h - WINDOWPADDING * 2 - 40);

    }

}

void place_widget(struct widget *widget, int x, int y, unsigned int w, unsigned int h)
{

    switch (widget->type)
    {

    case WIDGET_TYPE_BUTTON:
        placebutton(widget, x, y, w, h);

        break;

    case WIDGET_TYPE_CONTAINER:
        placecontainer(widget, x, y, w, h);

        break;

    case WIDGET_TYPE_FILL:
        placefill(widget, x, y, w, h);

        break;

    case WIDGET_TYPE_IMAGE:
        placeimage(widget, x, y, w, h);

        break;

    case WIDGET_TYPE_TEXTBOX:
        placetextbox(widget, x, y, w, h);

        break;

    case WIDGET_TYPE_WINDOW:
        placewindow(widget, x, y, w, h);

        break;

    }

}

