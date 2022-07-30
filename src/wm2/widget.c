#include <fudge.h>
#include "widget.h"

struct token
{

    unsigned int key;
    char *value;

};

static struct token commands[] = {
    {WIDGET_COMMAND_NONE, ""},
    {WIDGET_COMMAND_COMMENT, "#"},
    {WIDGET_COMMAND_DELETE, "-"},
    {WIDGET_COMMAND_INSERT, "+"},
    {WIDGET_COMMAND_UPDATE, "="}
};

static struct token types[] =
{
    {WIDGET_TYPE_BUTTON, "button"},
    {WIDGET_TYPE_FILL, "fill"},
    {WIDGET_TYPE_GRID, "grid"},
    {WIDGET_TYPE_IMAGE, "image"},
    {WIDGET_TYPE_CONTAINER, "container"},
    {WIDGET_TYPE_TEXT, "text"},
    {WIDGET_TYPE_TEXTBOX, "textbox"},
    {WIDGET_TYPE_WINDOW, "window"}
};

static struct token attributes[] =
{
    {WIDGET_ATTR_COLOR, "color"},
    {WIDGET_ATTR_COLUMNS, "columns"},
    {WIDGET_ATTR_CONTENT, "content"},
    {WIDGET_ATTR_ID, "id"},
    {WIDGET_ATTR_IN, "in"},
    {WIDGET_ATTR_LABEL, "label"},
    {WIDGET_ATTR_LAYOUT, "layout"},
    {WIDGET_ATTR_MODE, "mode"},
    {WIDGET_ATTR_PADDING, "padding"},
    {WIDGET_ATTR_PLACEMENT, "placement"},
    {WIDGET_ATTR_TITLE, "title"}
};

static struct token containerlayouts[] =
{
    {WIDGET_CONTAINER_LAYOUT_FLOAT, "float"},
    {WIDGET_CONTAINER_LAYOUT_MAXIMIZE, "maximize"},
    {WIDGET_CONTAINER_LAYOUT_HORIZONTAL, "horizontal"},
    {WIDGET_CONTAINER_LAYOUT_VERTICAL, "vertical"}
};

static struct token containerplacements[] =
{
    {WIDGET_CONTAINER_PLACEMENT_NORMAL, "normal"},
    {WIDGET_CONTAINER_PLACEMENT_STRETCHED, "stretched"}
};

static struct token gridplacements[] =
{
    {WIDGET_GRID_PLACEMENT_NORMAL, "normal"},
    {WIDGET_GRID_PLACEMENT_STRETCHED, "stretched"}
};

static struct token textboxmodes[] =
{
    {WIDGET_TEXTBOX_MODE_NORMAL, "normal"},
    {WIDGET_TEXTBOX_MODE_READONLY, "readonly"}
};

static unsigned int getkey(struct token *tokens, unsigned int n, char *value)
{

    unsigned int i;

    for (i = 0; i < n; i++)
    {

        if (cstring_match(tokens[i].value, value))
            return tokens[i].key;

    }

    return 0;

}

void widget_setimage(struct widget_image *image, void *data, void *cmap)
{

    image->data = data;
    image->cmap = cmap;

}

static void setattributebutton(struct widget *widget, unsigned int attribute, char *value)
{

    struct widget_button *button = widget->data;

    switch (attribute)
    {

    case WIDGET_ATTR_LABEL:
        button->label = value;

        break;

    }

}

static void setattributecontainer(struct widget *widget, unsigned int attribute, char *value)
{

    struct widget_container *container = widget->data;

    switch (attribute)
    {

    case WIDGET_ATTR_LAYOUT:
        container->layout = getkey(containerlayouts, 4, value);

        break;

    case WIDGET_ATTR_PADDING:
        container->padding = cstring_rvalue(value, cstring_length(value), 10);

        break;

    case WIDGET_ATTR_PLACEMENT:
        container->placement = getkey(containerplacements, 2, value);

        break;

    }

}

static void setattributefill(struct widget *widget, unsigned int attribute, char *value)
{

    struct widget_fill *fill = widget->data;

    switch (attribute)
    {

    case WIDGET_ATTR_COLOR:
        fill->color = cstring_rvalue(value, cstring_length(value), 16);

        break;

    }

}

static void setattributegrid(struct widget *widget, unsigned int attribute, char *value)
{

    struct widget_grid *grid = widget->data;

    switch (attribute)
    {

    case WIDGET_ATTR_COLUMNS:
        grid->columns = cstring_rvalue(value, cstring_length(value), 10);

        break;

    case WIDGET_ATTR_PADDING:
        grid->padding = cstring_rvalue(value, cstring_length(value), 10);

        break;

    case WIDGET_ATTR_PLACEMENT:
        grid->placement = getkey(gridplacements, 2, value);

        break;

    }

}

static void setattributeimage(struct widget *widget, unsigned int attribute, char *value)
{

}

static void setattributetext(struct widget *widget, unsigned int attribute, char *value)
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

static void setattributetextbox(struct widget *widget, unsigned int attribute, char *value)
{

    struct widget_textbox *textbox = widget->data;

    switch (attribute)
    {

    case WIDGET_ATTR_CONTENT:
        textbox->length = cstring_length(value);
        textbox->content = value;

        break;

    case WIDGET_ATTR_MODE:
        textbox->mode = getkey(textboxmodes, 2, value);

        break;

    }

}

static void setattributewindow(struct widget *widget, unsigned int attribute, char *value)
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
        setattributebutton(widget, attribute, value);

        break;

    case WIDGET_TYPE_CONTAINER:
        setattributecontainer(widget, attribute, value);

        break;

    case WIDGET_TYPE_FILL:
        setattributefill(widget, attribute, value);

        break;

    case WIDGET_TYPE_GRID:
        setattributegrid(widget, attribute, value);

        break;

    case WIDGET_TYPE_IMAGE:
        setattributeimage(widget, attribute, value);

        break;

    case WIDGET_TYPE_TEXT:
        setattributetext(widget, attribute, value);

        break;

    case WIDGET_TYPE_TEXTBOX:
        setattributetextbox(widget, attribute, value);

        break;

    case WIDGET_TYPE_WINDOW:
        setattributewindow(widget, attribute, value);

        break;

    }

}

unsigned int widget_getcommand(char *value)
{

    return getkey(commands, 5, value);

}

unsigned int widget_gettype(char *value)
{

    return getkey(types, 8, value);

}

unsigned int widget_getattribute(char *value)
{

    return getkey(attributes, 11, value);

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

