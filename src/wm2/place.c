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

static void placebutton(struct widget *widget, int x, int y, unsigned int wmin, unsigned int hmin, unsigned int wmax, unsigned int hmax)
{

    struct widget_button *button = widget->data;
    unsigned int length = cstring_length(button->label);

    widget->position.x = x;
    widget->position.y = y;
    widget->size.w = render_getrowwidth(RENDER_FONTBOLD, button->label, length) + BUTTONPADDINGWIDTH * 2;
    widget->size.h = render_getrowheight(RENDER_FONTBOLD, button->label, length) + BUTTONPADDINGHEIGHT * 2;
    widget->size.w = util_clamp(widget->size.w, wmin, wmax);
    widget->size.h = util_clamp(widget->size.h, hmin, hmax);

}

static void placecontainerfloat(struct widget *widget, int x, int y, unsigned int wmin, unsigned int hmin, unsigned int wmax, unsigned int hmax)
{

    struct list_item *current = 0;

    while ((current = pool_nextin(current, widget->id)))
    {

        struct widget *child = current->data;

        place_widget(child, x, y, 0, 0, wmax, hmax);

    }

    widget->position.x = x;
    widget->position.y = y;
    widget->size.w = 0;
    widget->size.h = 0;
    widget->size.w = util_clamp(widget->size.w, wmin, wmax);
    widget->size.h = util_clamp(widget->size.h, hmin, hmax);

}

static void placecontainerhorizontal(struct widget *widget, int x, int y, unsigned int wmin, unsigned int hmin, unsigned int wmax, unsigned int hmax)
{

    struct widget_container *container = widget->data;
    struct list_item *current = 0;
    int offsetw = 0;
    int offseth = 0;
    int maxh = 0;

    while ((current = pool_nextin(current, widget->id)))
    {

        struct widget *child = current->data;
        int childx = x + container->padding + offsetw; 
        int childy = y + container->padding + offseth; 
        int childminw = 0;
        int childminh = 0;
        int childmaxw = wmax - container->padding * 2 - offsetw;
        int childmaxh = hmax - container->padding * 2 - offseth;

        if (container->placement == CONTAINER_PLACEMENT_STRETCHED)
            childminh = childmaxh;

        place_widget(child, childx, childy, childminw, childminh, childmaxw, childmaxh);

        offsetw += child->size.w + container->padding;
        maxh = util_max(maxh, child->size.h);

    }

    widget->position.x = x;
    widget->position.y = y;
    widget->size.w = offsetw;
    widget->size.h = maxh;
    widget->size.w = util_clamp(widget->size.w, wmin, wmax);
    widget->size.h = util_clamp(widget->size.h, hmin, hmax);

}

static void placecontainervertical(struct widget *widget, int x, int y, unsigned int wmin, unsigned int hmin, unsigned int wmax, unsigned int hmax)
{

    struct widget_container *container = widget->data;
    struct list_item *current = 0;
    int offsetw = 0;
    int offseth = 0;
    int maxw = 0;

    while ((current = pool_nextin(current, widget->id)))
    {

        struct widget *child = current->data;
        int childx = x + container->padding + offsetw; 
        int childy = y + container->padding + offseth; 
        int childminw = 0;
        int childminh = 0;
        int childmaxw = wmax - container->padding * 2 - offsetw;
        int childmaxh = hmax - container->padding * 2 - offseth;

        if (container->placement == CONTAINER_PLACEMENT_STRETCHED)
            childminw = childmaxw;

        place_widget(child, childx, childy, childminw, childminh, childmaxw, childmaxh);

        offseth += child->size.h + container->padding;
        maxw = util_max(maxw, child->size.w);

    }

    widget->position.x = x;
    widget->position.y = y;
    widget->size.w = maxw;
    widget->size.h = offseth;
    widget->size.w = util_clamp(widget->size.w, wmin, wmax);
    widget->size.h = util_clamp(widget->size.h, hmin, hmax);

}

static void placecontainer(struct widget *widget, int x, int y, unsigned int wmin, unsigned int hmin, unsigned int wmax, unsigned int hmax)
{

    struct widget_container *container = widget->data;

    switch (container->layout)
    {

    case CONTAINER_LAYOUT_FLOAT:
        placecontainerfloat(widget, x, y, wmin, hmin, wmax, hmax);
        
        break;

    case CONTAINER_LAYOUT_HORIZONTAL:
        placecontainerhorizontal(widget, x, y, wmin, hmin, wmax, hmax);

        break;

    case CONTAINER_LAYOUT_VERTICAL:
        placecontainervertical(widget, x, y, wmin, hmin, wmax, hmax);

        break;

    }

}

static void placefill(struct widget *widget, int x, int y, unsigned int wmin, unsigned int hmin, unsigned int wmax, unsigned int hmax)
{

    widget->position.x = x;
    widget->position.y = y;
    widget->size.w = wmax;
    widget->size.h = hmax;
    widget->size.w = util_clamp(widget->size.w, wmin, wmax);
    widget->size.h = util_clamp(widget->size.h, hmin, hmax);

}

static void placegrid(struct widget *widget, int x, int y, unsigned int wmin, unsigned int hmin, unsigned int wmax, unsigned int hmax)
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
    unsigned int colw = (wmax - grid->padding * (grid->columns + 1)) / grid->columns;

    while ((current = pool_nextin(current, widget->id)))
    {

        struct widget *child = current->data;
        int childx = x + grid->padding + offsetw; 
        int childy = y + grid->padding + offseth; 
        int childminw = 0;
        int childminh = 0;
        int childmaxw = wmax - grid->padding * 2 - offsetw;
        int childmaxh = hmax - grid->padding * 2 - rowh - offseth;

        if (grid->placement == GRID_PLACEMENT_STRETCHED)
        {

            childminw = colw;
            childmaxw = colw;

        }

        place_widget(child, childx, childy, childminw, childminh, childmaxw, childmaxh);

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
    widget->size.w = util_clamp(widget->size.w, maxw, wmax);
    widget->size.h = util_clamp(widget->size.h, maxh, hmax);
    widget->size.w = util_clamp(widget->size.w, wmin, wmax);
    widget->size.h = util_clamp(widget->size.h, hmin, hmax);

}

static void placeimage(struct widget *widget, int x, int y, unsigned int wmin, unsigned int hmin, unsigned int wmax, unsigned int hmax)
{

}

static void placetext(struct widget *widget, int x, int y, unsigned int wmin, unsigned int hmin, unsigned int wmax, unsigned int hmax)
{

    struct widget_text *text = widget->data;

    widget->position.x = x;
    widget->position.y = y;
    widget->size.w = render_gettextwidth(RENDER_FONTNORMAL, text->content, text->length);
    widget->size.h = render_gettextheight(RENDER_FONTNORMAL, text->content, text->length, 1);
    widget->size.w = util_clamp(widget->size.w, wmin, wmax);
    widget->size.h = util_clamp(widget->size.h, hmin, hmax);

}

static void placetextbox(struct widget *widget, int x, int y, unsigned int wmin, unsigned int hmin, unsigned int wmax, unsigned int hmax)
{

    struct widget_textbox *textbox = widget->data;

    widget->position.x = x;
    widget->position.y = y;
    widget->size.w = render_gettextwidth(RENDER_FONTNORMAL, textbox->content, textbox->length) + TEXTBOXPADDING * 2;
    widget->size.h = render_gettextheight(RENDER_FONTNORMAL, textbox->content, textbox->length, 1) + TEXTBOXPADDING * 2;
    widget->size.w = util_clamp(widget->size.w, wmin, wmax);
    widget->size.h = util_clamp(widget->size.h, hmin, hmax);

}

static void placewindow(struct widget *widget, int x, int y, unsigned int wmin, unsigned int hmin, unsigned int wmax, unsigned int hmax)
{

    struct list_item *current = 0;

    while ((current = pool_nextin(current, widget->id)))
    {

        struct widget *child = current->data;
        int childx = widget->position.x + 5;
        int childy = widget->position.y + 41;
        int childminw = 0;
        int childminh = 0;
        int childmaxw = widget->size.w - 10;
        int childmaxh = widget->size.h - 46;

        place_widget(child, childx, childy, childminw, childminh, childmaxw, childmaxh);

    }

    widget->size.w = util_clamp(widget->size.w, wmin, wmax);
    widget->size.h = util_clamp(widget->size.h, hmin, hmax);

}

void place_widget(struct widget *widget, int x, int y, unsigned int wmin, unsigned int hmin, unsigned int wmax, unsigned int hmax)
{

    switch (widget->type)
    {

    case WIDGET_TYPE_BUTTON:
        placebutton(widget, x, y, wmin, hmin, wmax, hmax);

        break;

    case WIDGET_TYPE_CONTAINER:
        placecontainer(widget, x, y, wmin, hmin, wmax, hmax);

        break;

    case WIDGET_TYPE_FILL:
        placefill(widget, x, y, wmin, hmin, wmax, hmax);

        break;

    case WIDGET_TYPE_GRID:
        placegrid(widget, x, y, wmin, hmin, wmax, hmax);

        break;

    case WIDGET_TYPE_IMAGE:
        placeimage(widget, x, y, wmin, hmin, wmax, hmax);

        break;

    case WIDGET_TYPE_TEXT:
        placetext(widget, x, y, wmin, hmin, wmax, hmax);

        break;

    case WIDGET_TYPE_TEXTBOX:
        placetextbox(widget, x, y, wmin, hmin, wmax, hmax);

        break;

    case WIDGET_TYPE_WINDOW:
        placewindow(widget, x, y, wmin, hmin, wmax, hmax);

        break;

    }

}

