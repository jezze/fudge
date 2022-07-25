#include <fudge.h>
#include <abi.h>
#include "util.h"
#include "widget.h"
#include "pool.h"
#include "place.h"
#include "render.h"

#define BUTTONPADDINGWIDTH              48
#define BUTTONPADDINGHEIGHT             24
#define WINDOWPADDING                   24

static void placebutton(struct widget *widget, int x, int y, unsigned int w, unsigned int h)
{

    struct widget_button *button = widget->data;
    unsigned int length = cstring_length(button->label);

    widget->position.x = x;
    widget->position.y = y;
    widget->size.w = render_getrowwidth(button->label, length) + BUTTONPADDINGWIDTH * 2;
    widget->size.h = render_getrowheight(button->label, length) + BUTTONPADDINGHEIGHT * 2;

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
    widget->size.w = 0;
    widget->size.h = 0;

}

static void placecontainerhorizontal(struct widget *widget, int x, int y, unsigned int w, unsigned int h)
{

    struct list_item *current = 0;
    int offsetw = 0;
    int maxh = 0;

    while ((current = pool_nextin(current, widget->id)))
    {

        struct widget *child = current->data;

        place_widget(child, x + offsetw, y, w - offsetw, h);

        child->size.w = util_clamp(child->size.w, 0, w - offsetw);
        child->size.h = util_clamp(child->size.h, 0, h);
        offsetw += child->size.w;
        maxh = util_max(maxh, child->size.h);

    }

    widget->position.x = x;
    widget->position.y = y;
    widget->size.w = util_clamp(offsetw, 0, w);
    widget->size.h = maxh;

}

static void placecontainervertical(struct widget *widget, int x, int y, unsigned int w, unsigned int h)
{

    struct list_item *current = 0;
    int offseth = 0;
    int maxw = 0;

    while ((current = pool_nextin(current, widget->id)))
    {

        struct widget *child = current->data;

        place_widget(child, x, y + offseth, w, h - offseth);

        child->size.w = util_clamp(child->size.w, 0, w);
        child->size.h = util_clamp(child->size.h, 0, h - offseth);
        offseth += child->size.h;
        maxw = util_max(maxw, child->size.w);

    }

    widget->position.x = x;
    widget->position.y = y;
    widget->size.w = maxw;
    widget->size.h = util_clamp(offseth, 0, h);

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

    struct widget_textbox *textbox = widget->data;

    widget->position.x = x;
    widget->position.y = y;
    widget->size.w = render_gettextwidth(textbox->content, textbox->length);
    widget->size.h = 400;

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

