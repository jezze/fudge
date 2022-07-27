#include <fudge.h>
#include <abi.h>
#include "util.h"
#include "widget.h"
#include "pool.h"
#include "place.h"
#include "render.h"

#define BUTTONPADDINGWIDTH              48
#define BUTTONPADDINGHEIGHT             24
#define TEXTBOXPADDING                  32

static void placebutton(struct widget *widget, int x, int y, unsigned int wmax, unsigned int hmax)
{

    struct widget_button *button = widget->data;
    unsigned int length = cstring_length(button->label);

    widget->position.x = x;
    widget->position.y = y;
    widget->size.w = render_getrowwidth(button->label, length) + BUTTONPADDINGWIDTH * 2;
    widget->size.h = render_getrowheight(button->label, length) + BUTTONPADDINGHEIGHT * 2;

}

static void placecontainerfloat(struct widget *widget, int x, int y, unsigned int wmax, unsigned int hmax)
{

    struct list_item *current = 0;

    while ((current = pool_nextin(current, widget->id)))
    {

        struct widget *child = current->data;

        place_widget(child, x, y, wmax, hmax);

    }

    widget->position.x = x;
    widget->position.y = y;
    widget->size.w = 0;
    widget->size.h = 0;

}

static void placecontainerhorizontal(struct widget *widget, int x, int y, unsigned int wmax, unsigned int hmax)
{

    struct widget_container *container = widget->data;
    struct list_item *current = 0;
    int offsetw = 0;
    int maxh = 0;

    while ((current = pool_nextin(current, widget->id)))
    {

        struct widget *child = current->data;

        place_widget(child, x + container->padding + offsetw, y + container->padding, wmax - offsetw, hmax);

        child->size.w = util_clamp(child->size.w, 0, wmax - offsetw);
        child->size.h = util_clamp(child->size.h, 0, hmax);
        offsetw += child->size.w + container->padding;
        maxh = util_max(maxh, child->size.h);

    }

    widget->position.x = x;
    widget->position.y = y;
    widget->size.w = util_clamp(offsetw, 0, wmax);
    widget->size.h = maxh;

}

static void placecontainervertical(struct widget *widget, int x, int y, unsigned int wmax, unsigned int hmax)
{

    struct widget_container *container = widget->data;
    struct list_item *current = 0;
    int offseth = 0;
    int maxw = 0;

    while ((current = pool_nextin(current, widget->id)))
    {

        struct widget *child = current->data;

        place_widget(child, x + container->padding, y + container->padding + offseth, wmax, hmax - offseth);

        child->size.w = util_clamp(child->size.w, 0, wmax);
        child->size.h = util_clamp(child->size.h, 0, hmax - offseth);
        offseth += child->size.h + container->padding;
        maxw = util_max(maxw, child->size.w);

    }

    widget->position.x = x;
    widget->position.y = y;
    widget->size.w = maxw;
    widget->size.h = util_clamp(offseth, 0, hmax);

}

static void placecontainer(struct widget *widget, int x, int y, unsigned int wmax, unsigned int hmax)
{

    struct widget_container *container = widget->data;

    switch (container->layout)
    {

    case CONTAINER_LAYOUT_FLOAT:
        placecontainerfloat(widget, x, y, wmax, hmax);
        
        break;

    case CONTAINER_LAYOUT_HORIZONTAL:
        placecontainerhorizontal(widget, x, y, wmax, hmax);

        break;

    case CONTAINER_LAYOUT_VERTICAL:
        placecontainervertical(widget, x, y, wmax, hmax);

        break;

    }

}

static void placefill(struct widget *widget, int x, int y, unsigned int wmax, unsigned int hmax)
{

    widget->position.x = x;
    widget->position.y = y;
    widget->size.w = wmax;
    widget->size.h = hmax;

}

static void placegrid(struct widget *widget, int x, int y, unsigned int wmax, unsigned int hmax)
{

    struct widget_grid *grid = widget->data;
    struct list_item *current = 0;
    int offsetw = 0;
    int offseth = 0;
    int roww = 0;
    int rowh = 0;
    int maxw = 0;
    int maxh = 0;
    unsigned int num = 0;

    while ((current = pool_nextin(current, widget->id)))
    {

        struct widget *child = current->data;

        place_widget(child, x + grid->padding + offsetw, y + grid->padding + offseth, wmax - offsetw, hmax - rowh - offseth);

        child->size.w = util_clamp(child->size.w, 0, wmax - offsetw);
        child->size.h = util_clamp(child->size.h, 0, hmax - offseth);
        offsetw += child->size.w + grid->padding;
        rowh = util_max(rowh, child->size.h);
        maxh = util_max(maxh, rowh);

        num++;

        if (num % grid->columns == 0)
        {

            maxw = util_max(maxw, roww);
            offseth += rowh + grid->padding;
            offsetw = 0;
            roww = 0;
            rowh = 0;

        }

    }

    widget->position.x = x;
    widget->position.y = y;
    widget->size.w = maxw;
    widget->size.h = maxh;

}

static void placeimage(struct widget *widget, int x, int y, unsigned int wmax, unsigned int hmax)
{

}

static void placetext(struct widget *widget, int x, int y, unsigned int wmax, unsigned int hmax)
{

    struct widget_text *text = widget->data;

    widget->position.x = x;
    widget->position.y = y;
    widget->size.w = render_gettextwidth(text->content, text->length);
    widget->size.h = render_gettextheight(text->content, text->length, 1);

}

static void placetextbox(struct widget *widget, int x, int y, unsigned int wmax, unsigned int hmax)
{

    struct widget_textbox *textbox = widget->data;

    widget->position.x = x;
    widget->position.y = y;
    widget->size.w = render_gettextwidth(textbox->content, textbox->length) + TEXTBOXPADDING * 2;
    widget->size.h = render_gettextheight(textbox->content, textbox->length, 1) + TEXTBOXPADDING * 2;

}

static void placewindow(struct widget *widget, int x, int y, unsigned int wmax, unsigned int hmax)
{

    struct list_item *current = 0;

    while ((current = pool_nextin(current, widget->id)))
    {

        struct widget *child = current->data;

        place_widget(child, widget->position.x + 5, widget->position.y + 41, widget->size.w - 10, widget->size.h - (41 + 5));

    }

}

void place_widget(struct widget *widget, int x, int y, unsigned int wmax, unsigned int hmax)
{

    switch (widget->type)
    {

    case WIDGET_TYPE_BUTTON:
        placebutton(widget, x, y, wmax, hmax);

        break;

    case WIDGET_TYPE_CONTAINER:
        placecontainer(widget, x, y, wmax, hmax);

        break;

    case WIDGET_TYPE_FILL:
        placefill(widget, x, y, wmax, hmax);

        break;

    case WIDGET_TYPE_GRID:
        placegrid(widget, x, y, wmax, hmax);

        break;

    case WIDGET_TYPE_IMAGE:
        placeimage(widget, x, y, wmax, hmax);

        break;

    case WIDGET_TYPE_TEXT:
        placetext(widget, x, y, wmax, hmax);

        break;

    case WIDGET_TYPE_TEXTBOX:
        placetextbox(widget, x, y, wmax, hmax);

        break;

    case WIDGET_TYPE_WINDOW:
        placewindow(widget, x, y, wmax, hmax);

        break;

    }

}

