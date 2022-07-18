#include "widget.h"

void widget_init(struct widget *widget, unsigned int type, char *id, char *in, void *data)
{

    widget->type = type;
    widget->id = id;
    widget->in = in;
    widget->data = data;

}

void widget_initbutton(struct widget_button *button, char *label)
{

    button->position.x = 0;
    button->position.y = 0;
    button->size.w = 0;
    button->size.h = 0;
    button->label = label;
    button->focus = 0;

}

void widget_initimage(struct widget_image *image, void *data, void *cmap)
{

    image->position.x = 0;
    image->position.y = 0;
    image->size.w = 0;
    image->size.h = 0;
    image->data = data;
    image->cmap = cmap;

}

void widget_initlayout(struct widget_layout *layout, unsigned int type)
{

    layout->position.x = 0;
    layout->position.y = 0;
    layout->size.w = 0;
    layout->size.h = 0;
    layout->type = LAYOUT_TYPE_FLOAT;

}

void widget_initwindow(struct widget_window *window, char *title)
{

    window->position.x = 0;
    window->position.y = 0;
    window->size.w = 0;
    window->size.h = 0;
    window->title = title;
    window->focus = 0;

}

