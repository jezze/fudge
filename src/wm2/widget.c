#include <fudge.h>
#include "widget.h"

void widget_initcontainer(struct widget_container *container, unsigned int layout, unsigned int placement, unsigned int padding)
{

    container->layout = layout;
    container->placement = placement;
    container->padding = padding;

}

void widget_initgrid(struct widget_grid *grid, unsigned int columns, unsigned int placement, unsigned int padding)
{

    grid->columns = columns;
    grid->placement = placement;
    grid->padding = padding;

}

void widget_initimage(struct widget_image *image, void *data, void *cmap)
{

    image->data = data;
    image->cmap = cmap;

}

void widget_inittextbox(struct widget_textbox *textbox, unsigned int length, char *content, unsigned int mode)
{

    textbox->content = content;
    textbox->length = length;
    textbox->mode = mode;
    textbox->scroll = 0;
    textbox->cursor = 0;

}

static void widget_setattributebutton(struct widget *widget, unsigned int attribute, char *value)
{

    struct widget_button *button = widget->data;

    switch (attribute)
    {

    case WIDGET_ATTR_LABEL:
        button->label = value;

        break;

    }

}

static void widget_setattributefill(struct widget *widget, unsigned int attribute, char *value)
{

    struct widget_fill *fill = widget->data;

    switch (attribute)
    {

    case WIDGET_ATTR_COLOR:
        fill->color = cstring_rvalue(value, cstring_length(value), 16);

        break;

    }

}

static void widget_setattributetext(struct widget *widget, unsigned int attribute, char *value)
{

    struct widget_text *text = widget->data;

    switch (attribute)
    {

    case WIDGET_ATTR_CONTENT:
        text->length = cstring_length(value);
        text->content = value;

        break;

    }

}

static void widget_setattributetextbox(struct widget *widget, unsigned int attribute, char *value)
{

    struct widget_textbox *textbox = widget->data;

    switch (attribute)
    {

    case WIDGET_ATTR_CONTENT:
        textbox->length = cstring_length(value);
        textbox->content = value;

        break;

    }

}

static void widget_setattributewindow(struct widget *widget, unsigned int attribute, char *value)
{

    struct widget_window *window = widget->data;

    switch (attribute)
    {

    case WIDGET_ATTR_TITLE:
        window->title = value;

        break;

    }

}

void widget_setattribute(struct widget *widget, unsigned int attribute, char *value)
{

    switch (attribute)
    {

    case WIDGET_ATTR_ID:
        widget->id = value;

        break;

    case WIDGET_ATTR_IN:
        widget->in = value;

        break;

    }

    switch (widget->type)
    {

    case WIDGET_TYPE_BUTTON:
        widget_setattributebutton(widget, attribute, value);

        break;

    case WIDGET_TYPE_FILL:
        widget_setattributefill(widget, attribute, value);

        break;

    case WIDGET_TYPE_TEXT:
        widget_setattributetext(widget, attribute, value);

        break;

    case WIDGET_TYPE_TEXTBOX:
        widget_setattributetextbox(widget, attribute, value);

        break;

    case WIDGET_TYPE_WINDOW:
        widget_setattributewindow(widget, attribute, value);

        break;

    }

}

void widget_init(struct widget *widget, unsigned int source, unsigned int type, char *id, char *in, void *data)
{

    widget->source = source;
    widget->type = type;
    widget->id = id;
    widget->in = in;
    widget->data = data;
    widget->position.x = 0;
    widget->position.y = 0;
    widget->size.w = 0;
    widget->size.h = 0;

}

