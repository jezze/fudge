#include "widget.h"

void widget_initbutton(struct widget_button *button, char *label)
{

    button->label = label;
    button->focus = 0;

}

void widget_initcontainer(struct widget_container *container, unsigned int layout, unsigned int padding)
{

    container->layout = layout;
    container->padding = padding;

}

void widget_initfill(struct widget_fill *fill, unsigned int color)
{

    fill->color = color;

}

void widget_initgrid(struct widget_grid *grid, unsigned int columns, unsigned int padding)
{

    grid->columns = columns;
    grid->padding = padding;

}

void widget_initimage(struct widget_image *image, void *data, void *cmap)
{

    image->data = data;
    image->cmap = cmap;

}

void widget_inittext(struct widget_text *text, unsigned int length, char *content)
{

    text->content = content;
    text->length = length;
    text->scroll = 0;

}

void widget_inittextbox(struct widget_textbox *textbox, unsigned int length, char *content)
{

    textbox->content = content;
    textbox->length = length;
    textbox->scroll = 0;
    textbox->cursor = 0;

}

void widget_initwindow(struct widget_window *window, char *title, unsigned int focus)
{

    window->title = title;
    window->focus = focus;

}

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

