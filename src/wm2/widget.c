#include "widget.h"

void widget_init(struct widget *widget, unsigned int type, char *id, char *in, void *data)
{

    widget->type = type;
    widget->id = id;
    widget->in = in;
    widget->data = data;

}

void layout_init(struct layout *layout, unsigned int type)
{

    layout->type = LAYOUT_TYPE_FLOAT;
    layout->size.w = 0;
    layout->size.h = 0;
    layout->position.x = 0;
    layout->position.y = 0;

}

void window_init(struct window *window, char *title, int x, int y, unsigned int w, unsigned int h)
{

    window->title = title;
    window->position.x = x;
    window->position.y = y;
    window->size.w = w;
    window->size.h = h;
    window->focus = 0;

}

