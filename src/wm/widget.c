#include <fudge.h>
#include "config.h"
#include "util.h"
#include "text.h"
#include "widget.h"
#include "pool.h"

struct token
{

    unsigned int key;
    char *value;

};

static struct token commands[] =
{
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
    {WIDGET_TYPE_CHOICE, "choice"},
    {WIDGET_TYPE_LAYOUT, "layout"},
    {WIDGET_TYPE_SELECT, "select"},
    {WIDGET_TYPE_TEXT, "text"},
    {WIDGET_TYPE_TEXTBOX, "textbox"},
    {WIDGET_TYPE_TEXTBUTTON, "textbutton"},
    {WIDGET_TYPE_WINDOW, "window"}
};

static struct token attributes[] =
{
    {WIDGET_ATTR_COLOR, "color"},
    {WIDGET_ATTR_COLUMNS, "columns"},
    {WIDGET_ATTR_CONTENT, "content"},
    {WIDGET_ATTR_HALIGN, "halign"},
    {WIDGET_ATTR_ID, "id"},
    {WIDGET_ATTR_IN, "in"},
    {WIDGET_ATTR_LABEL, "label"},
    {WIDGET_ATTR_MODE, "mode"},
    {WIDGET_ATTR_PADDING, "padding"},
    {WIDGET_ATTR_PLACEMENT, "placement"},
    {WIDGET_ATTR_SOURCE, "source"},
    {WIDGET_ATTR_TITLE, "title"},
    {WIDGET_ATTR_TYPE, "type"},
    {WIDGET_ATTR_WEIGHT, "weight"},
    {WIDGET_ATTR_VALIGN, "valign"},
    {WIDGET_ATTR_WRAP, "wrap"}
};

static struct token layouttypes[] =
{
    {LAYOUT_TYPE_FLOAT, "float"},
    {LAYOUT_TYPE_MAXIMIZE, "maximize"},
    {LAYOUT_TYPE_HORIZONTAL, "horizontal"},
    {LAYOUT_TYPE_VERTICAL, "vertical"}
};

static struct token layoutplacements[] =
{
    {LAYOUT_PLACEMENT_NORMAL, "normal"},
    {LAYOUT_PLACEMENT_STRETCHED, "stretched"}
};

static struct token gridplacements[] =
{
    {GRID_PLACEMENT_NORMAL, "normal"},
    {GRID_PLACEMENT_STRETCHED, "stretched"}
};

static struct token imagetypes[] =
{
    {IMAGE_TYPE_FUDGEMOUSE, "image/fudge-icon-mouse"},
    {IMAGE_TYPE_PCX, "image/pcx"}
};

static struct token texthaligns[] =
{
    {TEXT_HALIGN_LEFT, "left"},
    {TEXT_HALIGN_CENTER, "center"},
    {TEXT_HALIGN_RIGHT, "right"}
};

static struct token textweights[] =
{
    {TEXT_WEIGHT_NORMAL, "normal"},
    {TEXT_WEIGHT_BOLD, "bold"}
};

static struct token textvaligns[] =
{
    {TEXT_VALIGN_TOP, "top"},
    {TEXT_VALIGN_MIDDLE, "middle"},
    {TEXT_VALIGN_BOTTOM, "bottom"}
};

static struct token textwraps[] =
{
    {TEXT_WRAP_NONE, "none"},
    {TEXT_WRAP_CHAR, "char"},
    {TEXT_WRAP_WORD, "word"}
};

static struct token textmodes[] =
{
    {TEXT_MODE_NORMAL, "normal"},
    {TEXT_MODE_INVERTED, "inverted"}
};

static struct token textboxmodes[] =
{
    {TEXTBOX_MODE_NORMAL, "normal"},
    {TEXTBOX_MODE_READONLY, "readonly"},
    {TEXTBOX_MODE_SELECT, "select"}
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

static void setattributebutton(struct widget *widget, unsigned int attribute, char *value)
{

    struct widget_button *button = widget->data;

    switch (attribute)
    {

    case WIDGET_ATTR_LABEL:
        button->label = pool_updatestring(button->label, value);

        break;

    }

}

static void setattributechoice(struct widget *widget, unsigned int attribute, char *value)
{

    struct widget_choice *choice = widget->data;

    switch (attribute)
    {

    case WIDGET_ATTR_LABEL:
        choice->label = pool_updatestring(choice->label, value);

        break;

    }

}

static void setattributelayout(struct widget *widget, unsigned int attribute, char *value)
{

    struct widget_layout *layout = widget->data;

    switch (attribute)
    {

    case WIDGET_ATTR_TYPE:
        layout->type = getkey(layouttypes, 4, value);

        break;

    case WIDGET_ATTR_PADDING:
        layout->padding = cstring_readvalue(value, cstring_length(value), 10);

        break;

    case WIDGET_ATTR_PLACEMENT:
        layout->placement = getkey(layoutplacements, 2, value);

        break;

    }

}

static void setattributefill(struct widget *widget, unsigned int attribute, char *value)
{

    struct widget_fill *fill = widget->data;

    switch (attribute)
    {

    case WIDGET_ATTR_COLOR:
        fill->color = cstring_readvalue(value, cstring_length(value), 16);

        break;

    }

}

static void setattributegrid(struct widget *widget, unsigned int attribute, char *value)
{

    struct widget_grid *grid = widget->data;

    switch (attribute)
    {

    case WIDGET_ATTR_COLUMNS:
        grid->columns = cstring_readvalue(value, cstring_length(value), 10);

        break;

    case WIDGET_ATTR_PADDING:
        grid->padding = cstring_readvalue(value, cstring_length(value), 10);

        break;

    case WIDGET_ATTR_PLACEMENT:
        grid->placement = getkey(gridplacements, 2, value);

        break;

    }

}

static void setattributeimage(struct widget *widget, unsigned int attribute, char *value)
{

    struct widget_image *image = widget->data;

    switch (attribute)
    {

    case WIDGET_ATTR_SOURCE:
        image->source = pool_updatestring(image->source, value);

        break;

    case WIDGET_ATTR_TYPE:
        image->type = getkey(imagetypes, 2, value);

        break;

    }

}

static void setattributeselect(struct widget *widget, unsigned int attribute, char *value)
{

    struct widget_select *select = widget->data;

    switch (attribute)
    {

    case WIDGET_ATTR_LABEL:
        select->label = pool_updatestring(select->label, value);

        break;

    }

}

static void setattributetext(struct widget *widget, unsigned int attribute, char *value)
{

    struct widget_text *text = widget->data;

    switch (attribute)
    {

    case WIDGET_ATTR_HALIGN:
        text->halign = getkey(texthaligns, 3, value);

        break;

    case WIDGET_ATTR_CONTENT:
        text->content = pool_updatestring(text->content, value);

        break;

    case WIDGET_ATTR_MODE:
        text->mode = getkey(textmodes, 2, value);

        break;

    case WIDGET_ATTR_WEIGHT:
        text->weight = getkey(textweights, 2, value);

        break;

    case WIDGET_ATTR_VALIGN:
        text->valign = getkey(textvaligns, 3, value);

        break;

    case WIDGET_ATTR_WRAP:
        text->wrap = getkey(textwraps, 3, value);

        break;

    }

}

static void setattributetextbox(struct widget *widget, unsigned int attribute, char *value)
{

    struct widget_textbox *textbox = widget->data;

    switch (attribute)
    {

    case WIDGET_ATTR_MODE:
        textbox->mode = getkey(textboxmodes, 3, value);

        break;

    }

}

static void setattributetextbutton(struct widget *widget, unsigned int attribute, char *value)
{

    struct widget_textbutton *textbutton = widget->data;

    switch (attribute)
    {

    case WIDGET_ATTR_LABEL:
        textbutton->label = pool_updatestring(textbutton->label, value);

        break;

    }

}

static void setattributewindow(struct widget *widget, unsigned int attribute, char *value)
{

    struct widget_window *window = widget->data;

    switch (attribute)
    {

    case WIDGET_ATTR_TITLE:
        window->title = pool_updatestring(window->title, value);

        break;

    }

}

void widget_setattribute(struct widget *widget, unsigned int attribute, char *value)
{

    switch (attribute)
    {

    case WIDGET_ATTR_ID:
        widget->id = pool_updatestring(widget->id, value);

        break;

    case WIDGET_ATTR_IN:
        widget->in = pool_updatestring(widget->in, value);

        break;

    }

    switch (widget->type)
    {

    case WIDGET_TYPE_BUTTON:
        setattributebutton(widget, attribute, value);

        break;

    case WIDGET_TYPE_CHOICE:
        setattributechoice(widget, attribute, value);

        break;

    case WIDGET_TYPE_LAYOUT:
        setattributelayout(widget, attribute, value);

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

    case WIDGET_TYPE_SELECT:
        setattributeselect(widget, attribute, value);

        break;

    case WIDGET_TYPE_TEXT:
        setattributetext(widget, attribute, value);

        break;

    case WIDGET_TYPE_TEXTBOX:
        setattributetextbox(widget, attribute, value);

        break;

    case WIDGET_TYPE_TEXTBUTTON:
        setattributetextbutton(widget, attribute, value);

        break;

    case WIDGET_TYPE_WINDOW:
        setattributewindow(widget, attribute, value);

        break;

    }

}

void widget_unsetattributes(struct widget *widget)
{

    widget->id = pool_updatestring(widget->id, 0);
    widget->in = pool_updatestring(widget->in, 0);

    switch (widget->type)
    {

    case WIDGET_TYPE_BUTTON:
        setattributebutton(widget, WIDGET_ATTR_LABEL, 0);

        break;

    case WIDGET_TYPE_CHOICE:
        setattributechoice(widget, WIDGET_ATTR_LABEL, 0);

        break;

    case WIDGET_TYPE_IMAGE:
        setattributeimage(widget, WIDGET_ATTR_SOURCE, 0);

        break;

    case WIDGET_TYPE_SELECT:
        setattributeselect(widget, WIDGET_ATTR_LABEL, 0);

        break;

    case WIDGET_TYPE_TEXT:
        setattributetext(widget, WIDGET_ATTR_CONTENT, 0);

        break;

    case WIDGET_TYPE_TEXTBUTTON:
        setattributetextbutton(widget, WIDGET_ATTR_LABEL, 0);

        break;

    case WIDGET_TYPE_WINDOW:
        setattributewindow(widget, WIDGET_ATTR_TITLE, 0);

        break;

    }

}

unsigned int widget_getattribute(char *value)
{

    return getkey(attributes, 16, value);

}

unsigned int widget_getcommand(char *value)
{

    return getkey(commands, 5, value);

}

unsigned int widget_gettype(char *value)
{

    return getkey(types, 11, value);

}

unsigned int widget_setstate(struct widget *widget, unsigned int state)
{

    unsigned int oldstate = widget->state;

    switch (state)
    {

    case WIDGET_STATE_DESTROYED:
        widget->state = state;

        break;

    case WIDGET_STATE_FOCUS:
        widget->state = state;

        break;

    case WIDGET_STATE_FOCUSOFF:
        if (widget->state == WIDGET_STATE_FOCUS)
            widget->state = state;

        break;

    case WIDGET_STATE_HOVER:
        if (widget->state == WIDGET_STATE_NORMAL)
            widget->state = state;

        break;

    case WIDGET_STATE_HOVEROFF:
        if (widget->state == WIDGET_STATE_HOVER)
            widget->state = state;

        break;

    case WIDGET_STATE_NORMAL:
        if (widget->state == WIDGET_STATE_FOCUSOFF || widget->state == WIDGET_STATE_HOVEROFF)
            widget->state = state;

        break;

    default:
        widget->state = state;

        break;

    }

    return oldstate != state;

}

static void getclipping(struct widget *widget, struct widget_position *position, struct widget_size *size)
{

    switch (widget->type)
    {

    case WIDGET_TYPE_TEXTBOX:
        {

            struct widget_textbox *textbox = widget->data;

            if (textbox->mode == TEXTBOX_MODE_SELECT)
            {

                position->x = widget->position.x + 4;
                position->y = widget->position.y + 4;
                size->w = widget->size.w - 4 * 2;
                size->h = widget->size.h - 4 * 2;

            }

            else
            {

                position->x = widget->position.x + CONFIG_TEXTBOX_PADDING_WIDTH;
                position->y = widget->position.y + CONFIG_TEXTBOX_PADDING_HEIGHT;
                size->w = widget->size.w - CONFIG_TEXTBOX_PADDING_WIDTH * 2;
                size->h = widget->size.h - CONFIG_TEXTBOX_PADDING_HEIGHT * 2;

            }

        }

        break;

    default:
        position->x = widget->position.x;
        position->y = widget->position.y;
        size->w = widget->size.w;
        size->h = widget->size.h;

        break;

    }

}

unsigned int widget_intersectsx(struct widget *widget, int x)
{

    struct list_item *current = 0;
    struct widget *parent;
    struct widget_position position;
    struct widget_size size;

    switch (widget->type)
    {

    case WIDGET_TYPE_TEXT:
    case WIDGET_TYPE_TEXTBUTTON:
        parent = pool_getwidgetbyid(widget->source, pool_getstring(widget->in));

        getclipping(parent, &position, &size);

        return util_intersects(x, widget->position.x, widget->position.x + widget->size.w) && util_intersects(x, position.x, position.x + size.w);

    case WIDGET_TYPE_SELECT:
        switch (widget->state)
        {

        case WIDGET_STATE_FOCUS:
            if (util_intersects(x, widget->position.x, widget->position.x + widget->size.w))
                return 1;

            if (widget->state == WIDGET_STATE_FOCUS)
            {

                while ((current = pool_nextin(current, widget)))
                {

                    struct widget *child = current->data;

                    if (util_intersects(x, child->position.x, child->position.x + child->size.w))
                        return 1;

                }

            }

            break;

        default:
            return util_intersects(x, widget->position.x, widget->position.x + widget->size.w);

        }

    default:
        return util_intersects(x, widget->position.x, widget->position.x + widget->size.w);

    }

    return 0;

}

unsigned int widget_intersectsy(struct widget *widget, int y)
{

    struct list_item *current = 0;
    struct widget *parent;
    struct widget_position position;
    struct widget_size size;

    switch (widget->type)
    {

    case WIDGET_TYPE_TEXT:
    case WIDGET_TYPE_TEXTBUTTON:
        parent = pool_getwidgetbyid(widget->source, pool_getstring(widget->in));

        getclipping(parent, &position, &size);

        return util_intersects(y, widget->position.y, widget->position.y + widget->size.h) && util_intersects(y, position.y, position.y + size.h);

    case WIDGET_TYPE_SELECT:
        switch (widget->state)
        {

        case WIDGET_STATE_FOCUS:
            if (util_intersects(y, widget->position.y, widget->position.y + widget->size.h))
                return 1;

            if (widget->state == WIDGET_STATE_FOCUS)
            {

                while ((current = pool_nextin(current, widget)))
                {

                    struct widget *child = current->data;

                    if (util_intersects(y, child->position.y, child->position.y + child->size.h))
                        return 1;

                }

            }

            break;

        default:
            return util_intersects(y, widget->position.y, widget->position.y + widget->size.h);

        }

    default:
        return util_intersects(y, widget->position.y, widget->position.y + widget->size.h);

    }

    return 0;

}

unsigned int widget_intersects(struct widget *widget, int x, int y)
{

    return widget_intersectsx(widget, x) && widget_intersectsy(widget, y);

}

unsigned int widget_isdragable(struct widget *widget)
{

    switch (widget->type)
    {

    case WIDGET_TYPE_WINDOW:
        return 1;

    }

    return 0;

}

unsigned int widget_isinteractive(struct widget *widget)
{

    switch (widget->type)
    {

    case WIDGET_TYPE_BUTTON:
    case WIDGET_TYPE_CHOICE:
    case WIDGET_TYPE_SELECT:
    case WIDGET_TYPE_TEXTBOX:
    case WIDGET_TYPE_TEXTBUTTON:
    case WIDGET_TYPE_WINDOW:
        return 1;

    }

    return 0;

}

unsigned int widget_isscrollable(struct widget *widget)
{

    switch (widget->type)
    {

    case WIDGET_TYPE_TEXTBOX:
        return 1;

    }

    return 0;

}

void widget_initposition(struct widget_position *position, int x, int y)
{

    position->x = x;
    position->y = y;

}

void widget_initsize(struct widget_size *size, int w, int h)
{

    size->w = w;
    size->h = h;

}

void widget_init(struct widget *widget, unsigned int source, unsigned int type, char *id, char *in, void *data)
{

    widget_initposition(&widget->position, 0, 0);
    widget_initsize(&widget->size, 0, 0);

    widget->source = source;
    widget->type = type;
    widget->id = pool_updatestring(widget->id, id);
    widget->in = pool_updatestring(widget->in, in);
    widget->data = data;

}

