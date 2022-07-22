#include "widget.h"

void widget_init(struct widget *widget, unsigned int type, char *id, char *in, void *data)
{

    widget->type = type;
    widget->id = id;
    widget->in = in;
    widget->data = data;
    widget->position.x = 0;
    widget->position.y = 0;
    widget->size.w = 0;
    widget->size.h = 0;

}

void widget_initbutton(struct widget_button *button, char *label)
{

    button->label = label;
    button->focus = 0;

}

void widget_initfill(struct widget_fill *fill, unsigned int color)
{

    fill->color = color;

}

void widget_initimage(struct widget_image *image, void *data, void *cmap)
{

    image->data = data;
    image->cmap = cmap;

}

void widget_initcontainer(struct widget_container *container, unsigned int layout)
{

    container->layout = layout;

}

void widget_inittextbox(struct widget_textbox *textbox, unsigned int length, char *content)
{

    textbox->length = length;
    textbox->cursor = 0;
    textbox->offset = 0;
    textbox->scroll = 0;
    textbox->content = content;

}

void widget_initwindow(struct widget_window *window, char *title, unsigned int focus)
{

    window->title = title;
    window->focus = focus;

}

