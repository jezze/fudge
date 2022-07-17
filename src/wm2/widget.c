#include "widget.h"

void widget_init(struct widget *widget, unsigned int type, char *id, char *in, void *data)
{

    widget->type = type;
    widget->id = id;
    widget->in = in;
    widget->data = data;

}

void widget_initimage(struct widget_image *image, void *data, void *cmap)
{

    image->data = data;
    image->cmap = cmap;
    image->position.x = 0;
    image->position.y = 0;
    image->size.w = 0;
    image->size.h = 0;

}

void widget_initlayout(struct widget_layout *layout, unsigned int type)
{

    layout->type = LAYOUT_TYPE_FLOAT;
    layout->position.x = 0;
    layout->position.y = 0;
    layout->size.w = 0;
    layout->size.h = 0;

}

void widget_initwindow(struct widget_window *window, char *title, int x, int y, unsigned int w, unsigned int h)
{

    window->title = title;
    window->position.x = x;
    window->position.y = y;
    window->size.w = w;
    window->size.h = h;
    window->focus = 0;

}

