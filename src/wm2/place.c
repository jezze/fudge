#include <fudge.h>
#include <abi.h>
#include "util.h"
#include "widget.h"
#include "pool.h"
#include "place.h"
#include "render.h"

static void placebutton(struct widget *widget, int x, int y, unsigned int minw, unsigned int minh, unsigned int maxw, unsigned int maxh)
{

    struct widget_button *button = widget->data;
    struct render_rowinfo rowinfo;

    render_getrowinfo(RENDER_FONTBOLD, pool_getstring(button->label), pool_getcstringlength(button->label), &rowinfo, 0, 0, 0, 0);

    widget->position.x = x;
    widget->position.y = y;
    widget->size.w = util_clamp(rowinfo.width + RENDER_BUTTON_PADDING_WIDTH * 2, minw, maxw);
    widget->size.h = util_clamp(rowinfo.height + RENDER_BUTTON_PADDING_HEIGHT * 2, minh, maxh);

}

static void placecontainerfloat(struct widget *widget, int x, int y, unsigned int minw, unsigned int minh, unsigned int maxw, unsigned int maxh)
{

    struct list_item *current = 0;

    while ((current = pool_nextin(current, widget)))
    {

        struct widget *child = current->data;

        place_widget(child, x, y, 0, 0, maxw, maxh);

    }

    widget->position.x = x;
    widget->position.y = y;
    widget->size.w = util_clamp(0, minw, maxw);
    widget->size.h = util_clamp(0, minh, maxh);

}

static void placecontainerhorizontal(struct widget *widget, int x, int y, unsigned int minw, unsigned int minh, unsigned int maxw, unsigned int maxh)
{

    struct widget_container *container = widget->data;
    struct list_item *current = 0;
    int offw = 0;
    int offh = 0;
    int toth = 0;

    while ((current = pool_nextin(current, widget)))
    {

        struct widget *child = current->data;
        int childx = x + container->padding + offw; 
        int childy = y + container->padding + offh; 
        int childminw = 0;
        int childminh = 0;
        int childmaxw = maxw - container->padding * 2 - offw;
        int childmaxh = maxh - container->padding * 2 - offh;

        if (container->placement == WIDGET_CONTAINER_PLACEMENT_STRETCHED)
            childminh = childmaxh;

        place_widget(child, childx, childy, childminw, childminh, childmaxw, childmaxh);

        offw += child->size.w + container->padding;
        toth = util_max(toth, child->size.h);

    }

    widget->position.x = x;
    widget->position.y = y;
    widget->size.w = util_clamp(offw, minw, maxw);
    widget->size.h = util_clamp(toth, minh, maxh);

}

static void placecontainermaximize(struct widget *widget, int x, int y, unsigned int minw, unsigned int minh, unsigned int maxw, unsigned int maxh)
{

    struct widget_container *container = widget->data;
    struct list_item *current = 0;

    while ((current = pool_nextin(current, widget)))
    {

        struct widget *child = current->data;
        int childx = x + container->padding; 
        int childy = y + container->padding; 
        int childmaxw = maxw - container->padding * 2;
        int childmaxh = maxh - container->padding * 2;

        place_widget(child, childx, childy, childmaxw, childmaxh, childmaxw, childmaxh);

    }

    widget->position.x = x;
    widget->position.y = y;
    widget->size.w = util_clamp(maxw, minw, maxw);
    widget->size.h = util_clamp(maxh, minh, maxh);

}

static void placecontainervertical(struct widget *widget, int x, int y, unsigned int minw, unsigned int minh, unsigned int maxw, unsigned int maxh)
{

    struct widget_container *container = widget->data;
    struct list_item *current = 0;
    int offw = 0;
    int offh = 0;
    int totw = 0;

    while ((current = pool_nextin(current, widget)))
    {

        struct widget *child = current->data;
        int childx = x + container->padding + offw; 
        int childy = y + container->padding + offh; 
        int childminw = 0;
        int childminh = 0;
        int childmaxw = maxw - container->padding * 2 - offw;
        int childmaxh = maxh - container->padding * 2 - offh;

        if (container->placement == WIDGET_CONTAINER_PLACEMENT_STRETCHED)
            childminw = childmaxw;

        place_widget(child, childx, childy, childminw, childminh, childmaxw, childmaxh);

        offh += child->size.h + container->padding;
        totw = util_max(totw, child->size.w);

    }

    widget->position.x = x;
    widget->position.y = y;
    widget->size.w = util_clamp(totw, minw, maxw);
    widget->size.h = util_clamp(offh, minh, maxh);

}

static void placecontainer(struct widget *widget, int x, int y, unsigned int minw, unsigned int minh, unsigned int maxw, unsigned int maxh)
{

    struct widget_container *container = widget->data;

    switch (container->layout)
    {

    case WIDGET_CONTAINER_LAYOUT_FLOAT:
        placecontainerfloat(widget, x, y, minw, minh, maxw, maxh);
        
        break;

    case WIDGET_CONTAINER_LAYOUT_HORIZONTAL:
        placecontainerhorizontal(widget, x, y, minw, minh, maxw, maxh);

        break;

    case WIDGET_CONTAINER_LAYOUT_MAXIMIZE:
        placecontainermaximize(widget, x, y, minw, minh, maxw, maxh);

        break;

    case WIDGET_CONTAINER_LAYOUT_VERTICAL:
        placecontainervertical(widget, x, y, minw, minh, maxw, maxh);

        break;

    }

}

static void placefill(struct widget *widget, int x, int y, unsigned int minw, unsigned int minh, unsigned int maxw, unsigned int maxh)
{

    widget->position.x = x;
    widget->position.y = y;
    widget->size.w = util_clamp(maxw, minw, maxw);
    widget->size.h = util_clamp(maxh, minh, maxh);

}

static void placegrid(struct widget *widget, int x, int y, unsigned int minw, unsigned int minh, unsigned int maxw, unsigned int maxh)
{

    struct widget_grid *grid = widget->data;
    struct list_item *current = 0;
    int offw = 0;
    int offh = 0;
    int roww = 0;
    int rowh = 0;
    int totw = 0;
    int toth = 0;
    unsigned int numchildren = 0;
    unsigned int colw = (maxw - grid->padding * (grid->columns + 1)) / grid->columns;

    while ((current = pool_nextin(current, widget)))
    {

        struct widget *child = current->data;
        int childx = x + grid->padding + offw; 
        int childy = y + grid->padding + offh; 
        int childminw = 0;
        int childminh = 0;
        int childmaxw = maxw - grid->padding * 2 - offw;
        int childmaxh = maxh - grid->padding * 2 - offh;

        if (grid->placement == WIDGET_GRID_PLACEMENT_STRETCHED)
        {

            childminw = colw;
            childmaxw = colw;

        }

        place_widget(child, childx, childy, childminw, childminh, childmaxw, childmaxh);

        offw += colw + grid->padding;
        rowh = util_max(rowh, child->size.h);
        toth = util_max(toth, rowh);

        numchildren++;

        if (numchildren % grid->columns == 0)
        {

            totw = util_max(totw, roww);
            offh += rowh + grid->padding;
            offw = 0;
            roww = 0;
            rowh = 0;

        }

    }

    widget->position.x = x;
    widget->position.y = y;
    widget->size.w = util_clamp(totw, minw, maxw);
    widget->size.h = util_clamp(toth, minh, maxh);

}

static void placeimage(struct widget *widget, int x, int y, unsigned int minw, unsigned int minh, unsigned int maxw, unsigned int maxh)
{

}

static void placetext(struct widget *widget, int x, int y, unsigned int minw, unsigned int minh, unsigned int maxw, unsigned int maxh)
{

    struct widget_text *text = widget->data;
    struct render_textinfo textinfo;
    unsigned int index = (text->weight == WIDGET_TEXT_WEIGHT_BOLD) ? RENDER_FONTBOLD : RENDER_FONTNORMAL;

    render_gettextinfo(index, pool_getstring(text->content), pool_getcstringlength(text->content), &textinfo, text->wrap, 0, maxw);

    widget->position.x = x;
    widget->position.y = y;
    widget->size.w = util_clamp(textinfo.width, minw, maxw);
    widget->size.h = util_clamp(textinfo.rows * textinfo.lineheight, minh, maxh);

}

static void placetextbox(struct widget *widget, int x, int y, unsigned int minw, unsigned int minh, unsigned int maxw, unsigned int maxh)
{

    struct list_item *current = 0;
    int offw = 0;
    int offh = 0;
    int totw = 0;

    while ((current = pool_nextin(current, widget)))
    {

        struct widget *child = current->data;
        int childx = x + RENDER_TEXTBOX_PADDING_WIDTH + offw; 
        int childy = y + RENDER_TEXTBOX_PADDING_HEIGHT + offh; 
        int childminw = 0;
        int childminh = 0;
        int childmaxw = maxw - RENDER_TEXTBOX_PADDING_WIDTH * 2 - offw;
        int childmaxh = maxh - RENDER_TEXTBOX_PADDING_HEIGHT * 2 - offh;

        place_widget(child, childx, childy, childminw, childminh, childmaxw, childmaxh);

        offh += child->size.h + RENDER_TEXTBOX_PADDING_HEIGHT;
        totw = util_max(totw, child->size.w);

    }

    widget->position.x = x;
    widget->position.y = y;
    widget->size.w = util_clamp(totw, minw, maxw);
    widget->size.h = util_clamp(offh, minh, maxh);

}

static void placewindow(struct widget *widget, int x, int y, unsigned int minw, unsigned int minh, unsigned int maxw, unsigned int maxh)
{

    struct list_item *current = 0;

    while ((current = pool_nextin(current, widget)))
    {

        struct widget *child = current->data;
        int childx = widget->position.x + RENDER_WINDOW_BORDER_WIDTH;
        int childy = widget->position.y + RENDER_WINDOW_BORDER_HEIGHT + RENDER_WINDOW_TITLE_HEIGHT;
        int childminw = 0;
        int childminh = 0;
        int childmaxw = widget->size.w - RENDER_WINDOW_BORDER_WIDTH * 2;
        int childmaxh = widget->size.h - RENDER_WINDOW_BORDER_HEIGHT * 2 - RENDER_WINDOW_TITLE_HEIGHT;

        place_widget(child, childx, childy, childminw, childminh, childmaxw, childmaxh);

    }

    widget->size.w = util_clamp(widget->size.w, minw, maxw);
    widget->size.h = util_clamp(widget->size.h, minh, maxh);

}

void place_widget(struct widget *widget, int x, int y, unsigned int minw, unsigned int minh, unsigned int maxw, unsigned int maxh)
{

    switch (widget->type)
    {

    case WIDGET_TYPE_BUTTON:
        placebutton(widget, x, y, minw, minh, maxw, maxh);

        break;

    case WIDGET_TYPE_CONTAINER:
        placecontainer(widget, x, y, minw, minh, maxw, maxh);

        break;

    case WIDGET_TYPE_FILL:
        placefill(widget, x, y, minw, minh, maxw, maxh);

        break;

    case WIDGET_TYPE_GRID:
        placegrid(widget, x, y, minw, minh, maxw, maxh);

        break;

    case WIDGET_TYPE_IMAGE:
        placeimage(widget, x, y, minw, minh, maxw, maxh);

        break;

    case WIDGET_TYPE_TEXT:
        placetext(widget, x, y, minw, minh, maxw, maxh);

        break;

    case WIDGET_TYPE_TEXTBOX:
        placetextbox(widget, x, y, minw, minh, maxw, maxh);

        break;

    case WIDGET_TYPE_WINDOW:
        placewindow(widget, x, y, minw, minh, maxw, maxh);

        break;

    }

}

