#include <fudge.h>
#include "config.h"
#include "util.h"
#include "text.h"
#include "attr.h"
#include "widget.h"

static void setattributebutton(struct widget *widget, unsigned int attribute, char *value)
{

    struct widget_button *button = widget->data;

    switch (attribute)
    {

    case ATTR_LABEL:
        button->label = attr_update(ATTR_LABEL, value, button->label);

        break;

    case ATTR_ONCLICK:
        button->onclick = attr_update(ATTR_ONCLICK, value, button->onclick);

        break;

    }

}

static void setattributechoice(struct widget *widget, unsigned int attribute, char *value)
{

    struct widget_choice *choice = widget->data;

    switch (attribute)
    {

    case ATTR_LABEL:
        choice->label = attr_update(ATTR_LABEL, value, choice->label);

        break;

    case ATTR_ONCLICK:
        choice->onclick = attr_update(ATTR_ONCLICK, value, choice->onclick);

        break;

    }

}

static void setattributefill(struct widget *widget, unsigned int attribute, char *value)
{

    struct widget_fill *fill = widget->data;

    switch (attribute)
    {

    case ATTR_COLOR:
        fill->color = attr_update(ATTR_COLOR, value, fill->color);

        break;

    }

}

static void setattributeimage(struct widget *widget, unsigned int attribute, char *value)
{

    struct widget_image *image = widget->data;

    switch (attribute)
    {

    case ATTR_SOURCE:
        image->source = attr_update(ATTR_SOURCE, value, image->source);

        break;

    case ATTR_MIMETYPE:
        image->mimetype = attr_update(ATTR_MIMETYPE, value, image->mimetype);

        break;

    }

}

static void setattributelayout(struct widget *widget, unsigned int attribute, char *value)
{

    struct widget_layout *layout = widget->data;

    switch (attribute)
    {

    case ATTR_FLOW:
        layout->flow = attr_update(ATTR_FLOW, value, layout->flow);

        break;

    case ATTR_PADDING:
        layout->padding = attr_update(ATTR_PADDING, value, layout->padding);

        break;

    }

}

static void setattributelistbox(struct widget *widget, unsigned int attribute, char *value)
{

    struct widget_listbox *listbox = widget->data;

    switch (attribute)
    {

    case ATTR_MODE:
        listbox->mode = attr_update(ATTR_MODE, value, listbox->mode);

        break;

    case ATTR_OVERFLOW:
        listbox->overflow = attr_update(ATTR_OVERFLOW, value, listbox->overflow);

        break;

    }

}

static void setattributeselect(struct widget *widget, unsigned int attribute, char *value)
{

    struct widget_select *select = widget->data;

    switch (attribute)
    {

    case ATTR_LABEL:
        select->label = attr_update(ATTR_LABEL, value, select->label);

        break;

    case ATTR_ONCLICK:
        select->onclick = attr_update(ATTR_ONCLICK, value, select->onclick);

        break;

    }

}

static void setattributetext(struct widget *widget, unsigned int attribute, char *value)
{

    struct widget_text *text = widget->data;

    switch (attribute)
    {

    case ATTR_CONTENT:
        text->content = attr_update(ATTR_CONTENT, value, text->content);

        break;

    case ATTR_HALIGN:
        text->halign = attr_update(ATTR_HALIGN, value, text->halign);

        break;

    case ATTR_VALIGN:
        text->valign = attr_update(ATTR_VALIGN, value, text->valign);

        break;

    case ATTR_WEIGHT:
        text->weight = attr_update(ATTR_WEIGHT, value, text->weight);

        break;

    case ATTR_WRAP:
        text->wrap = attr_update(ATTR_WRAP, value, text->wrap);

        break;

    }

}

static void setattributetextbox(struct widget *widget, unsigned int attribute, char *value)
{

    struct widget_textbox *textbox = widget->data;

    switch (attribute)
    {

    case ATTR_CURSOR:
        textbox->cursor = attr_update(ATTR_CURSOR, value, textbox->cursor);
        textbox->enablecursor = 1; /* FIXME */

        break;

    case ATTR_MODE:
        textbox->mode = attr_update(ATTR_MODE, value, textbox->mode);

        break;

    case ATTR_OVERFLOW:
        textbox->overflow = attr_update(ATTR_OVERFLOW, value, textbox->overflow);

        break;

    }

}

static void setattributetextbutton(struct widget *widget, unsigned int attribute, char *value)
{

    struct widget_textbutton *textbutton = widget->data;

    switch (attribute)
    {

    case ATTR_LABEL:
        textbutton->label = attr_update(ATTR_LABEL, value, textbutton->label);

        break;

    case ATTR_ONCLICK:
        textbutton->onclick = attr_update(ATTR_ONCLICK, value, textbutton->onclick);

        break;

    }

}

static void setattributewindow(struct widget *widget, unsigned int attribute, char *value)
{

    struct widget_window *window = widget->data;

    switch (attribute)
    {

    case ATTR_TITLE:
        window->title = attr_update(ATTR_TITLE, value, window->title);

        break;

    }

}

void widget_setattribute(struct widget *widget, unsigned int attribute, char *value)
{

    switch (attribute)
    {

    case ATTR_ID:
        widget->id = attr_update(ATTR_ID, value, widget->id);

        break;

    case ATTR_IN:
        widget->in = attr_update(ATTR_IN, value, widget->in);

        break;

    case ATTR_SPAN:
        widget->span = attr_update(ATTR_SPAN, value, widget->span);

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

    case WIDGET_TYPE_FILL:
        setattributefill(widget, attribute, value);

        break;

    case WIDGET_TYPE_IMAGE:
        setattributeimage(widget, attribute, value);

        break;

    case WIDGET_TYPE_LAYOUT:
        setattributelayout(widget, attribute, value);

        break;

    case WIDGET_TYPE_LISTBOX:
        setattributelistbox(widget, attribute, value);

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

    widget->id = attr_update(ATTR_ID, 0, widget->id);
    widget->in = attr_update(ATTR_IN, 0, widget->in);

    switch (widget->type)
    {

    case WIDGET_TYPE_BUTTON:
        setattributebutton(widget, ATTR_LABEL, 0);
        setattributebutton(widget, ATTR_ONCLICK, 0);

        break;

    case WIDGET_TYPE_CHOICE:
        setattributechoice(widget, ATTR_LABEL, 0);
        setattributechoice(widget, ATTR_ONCLICK, 0);

        break;

    case WIDGET_TYPE_IMAGE:
        setattributeimage(widget, ATTR_SOURCE, 0);

        break;

    case WIDGET_TYPE_SELECT:
        setattributeselect(widget, ATTR_LABEL, 0);
        setattributeselect(widget, ATTR_ONCLICK, 0);

        break;

    case WIDGET_TYPE_TEXT:
        setattributetext(widget, ATTR_CONTENT, 0);

        break;

    case WIDGET_TYPE_TEXTBUTTON:
        setattributetextbutton(widget, ATTR_LABEL, 0);
        setattributetextbutton(widget, ATTR_ONCLICK, 0);

        break;

    case WIDGET_TYPE_WINDOW:
        setattributewindow(widget, ATTR_TITLE, 0);

        break;

    }

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

unsigned int widget_intersectsx(struct widget *widget, int x)
{

    return util_intersects(x, widget->bb.x, widget->bb.x + widget->bb.w) && util_intersects(x, widget->clip.x, widget->clip.x + widget->clip.w);

}

unsigned int widget_intersectsy(struct widget *widget, int y)
{

    return util_intersects(y, widget->bb.y, widget->bb.y + widget->bb.h) && util_intersects(y, widget->clip.y, widget->clip.y + widget->clip.h);

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

unsigned int widget_isresizable(struct widget *widget)
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
    case WIDGET_TYPE_LISTBOX:
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

    case WIDGET_TYPE_LISTBOX:
    case WIDGET_TYPE_TEXTBOX:
        return 1;

    }

    return 0;

}

static void initbutton(struct widget *widget)
{

    struct widget_button *button = widget->data;

    button->label = 0;
    button->onclick = 0;

}

static void initchoice(struct widget *widget)
{

    struct widget_choice *choice = widget->data;

    choice->label = 0;
    choice->onclick = 0;

}

static void initfill(struct widget *widget)
{

    struct widget_fill *fill = widget->data;

    fill->color = 0;

}

static void initimage(struct widget *widget)
{

    struct widget_image *image = widget->data;

    image->mimetype = ATTR_MIMETYPE_NONE;
    image->source = 0;
    image->loaded = 0;

    util_initsize(&image->size, 0, 0);

}

static void initlayout(struct widget *widget)
{

    struct widget_layout *layout = widget->data;

    layout->flow = ATTR_FLOW_DEFAULT;
    layout->padding = 0;

}

static void initlistbox(struct widget *widget)
{

    struct widget_listbox *listbox = widget->data;

    listbox->mode = ATTR_MODE_NORMAL;
    listbox->overflow = ATTR_OVERFLOW_NONE;
    listbox->hscroll = 0;
    listbox->vscroll = 0;

}

static void initselect(struct widget *widget)
{

    struct widget_select *select = widget->data;

    select->label = 0;
    select->onclick = 0;

}

static void inittext(struct widget *widget)
{

    struct widget_text *text = widget->data;

    text->content = 0;
    text->halign = ATTR_HALIGN_LEFT;
    text->valign = ATTR_VALIGN_TOP;
    text->weight = ATTR_WEIGHT_NORMAL;
    text->wrap = ATTR_WRAP_NONE;
    text->offx = 0;
    text->markstart = 0;
    text->markend = 0;
    text->rows = 0;
    text->lastrowx = 0;
    text->lastrowy = 0;

}

static void inittextbox(struct widget *widget)
{

    struct widget_textbox *textbox = widget->data;

    textbox->mode = ATTR_MODE_NORMAL;
    textbox->overflow = ATTR_OVERFLOW_NONE;
    textbox->hscroll = 0;
    textbox->vscroll = 0;
    textbox->cursorx = 0;
    textbox->cursory = 0;
    textbox->cursorheight = 0;
    textbox->cursorwidth = 0;

}

static void inittextbutton(struct widget *widget)
{

    struct widget_textbutton *textbutton = widget->data;

    textbutton->label = 0;
    textbutton->onclick = 0;

}

static void initwindow(struct widget *widget)
{

    struct widget_window *window = widget->data;

    window->title = 0;

}

void widget_init(struct widget *widget, unsigned int source, unsigned int type, char *id, char *in, void *data)
{

    widget->source = source;
    widget->type = type;
    widget->state = WIDGET_STATE_NORMAL;
    widget->span = 0;
    widget->id = attr_update(ATTR_ID, id, widget->id);
    widget->in = attr_update(ATTR_IN, in, widget->in);
    widget->data = data;

    util_initbox(&widget->bb, 0, 0, 0, 0);
    util_initbox(&widget->clip, 0, 0, 0, 0);

    switch (widget->type)
    {

    case WIDGET_TYPE_BUTTON:
        initbutton(widget);

        break;

    case WIDGET_TYPE_CHOICE:
        initchoice(widget);

        break;

    case WIDGET_TYPE_FILL:
        initfill(widget);

        break;

    case WIDGET_TYPE_IMAGE:
        initimage(widget);

        break;

    case WIDGET_TYPE_LAYOUT:
        initlayout(widget);

        break;

    case WIDGET_TYPE_LISTBOX:
        initlistbox(widget);

        break;

    case WIDGET_TYPE_SELECT:
        initselect(widget);

        break;

    case WIDGET_TYPE_TEXT:
        inittext(widget);

        break;

    case WIDGET_TYPE_TEXTBOX:
        inittextbox(widget);

        break;

    case WIDGET_TYPE_TEXTBUTTON:
        inittextbutton(widget);

        break;

    case WIDGET_TYPE_WINDOW:
        initwindow(widget);

        break;

    }

}

