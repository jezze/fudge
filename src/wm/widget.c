#include <fudge.h>
#include "config.h"
#include "util.h"
#include "text.h"
#include "attr.h"
#include "widget.h"

void widget_setattribute(struct widget *widget, unsigned int attribute, char *value)
{

    switch (attribute)
    {

    case ATTR_COLOR:
        widget->attributes.color = attr_update(ATTR_COLOR, value, widget->attributes.color);

        break;

    case ATTR_CURSOR:
        widget->attributes.cursor = attr_update(ATTR_CURSOR, value, widget->attributes.cursor);

        break;

    case ATTR_DISPLAY:
        widget->attributes.display = attr_update(ATTR_DISPLAY, value, widget->attributes.display);

        break;

    case ATTR_FLOW:
        widget->attributes.flow = attr_update(ATTR_FLOW, value, widget->attributes.flow);

        break;

    case ATTR_HALIGN:
        widget->attributes.halign = attr_update(ATTR_HALIGN, value, widget->attributes.halign);

        break;

    case ATTR_ID:
        widget->attributes.id = attr_update(ATTR_ID, value, widget->attributes.id);

        break;

    case ATTR_IN:
        widget->attributes.in = attr_update(ATTR_IN, value, widget->attributes.in);

        break;

    case ATTR_LABEL:
        widget->attributes.label = attr_update(ATTR_LABEL, value, widget->attributes.label);

        break;

    case ATTR_MIMETYPE:
        widget->attributes.mimetype = attr_update(ATTR_MIMETYPE, value, widget->attributes.mimetype);

        break;

    case ATTR_MODE:
        widget->attributes.mode = attr_update(ATTR_MODE, value, widget->attributes.mode);

        break;

    case ATTR_ONCLICK:
        widget->attributes.onclick = attr_update(ATTR_ONCLICK, value, widget->attributes.onclick);

        break;

    case ATTR_OVERFLOW:
        widget->attributes.overflow = attr_update(ATTR_OVERFLOW, value, widget->attributes.overflow);

        break;

    case ATTR_PADDING:
        widget->attributes.padding = attr_update(ATTR_PADDING, value, widget->attributes.padding);

        break;

    case ATTR_SOURCE:
        widget->attributes.source = attr_update(ATTR_SOURCE, value, widget->attributes.source);

        break;

    case ATTR_SPAN:
        widget->attributes.span = attr_update(ATTR_SPAN, value, widget->attributes.span);

        break;

    case ATTR_VALIGN:
        widget->attributes.valign = attr_update(ATTR_VALIGN, value, widget->attributes.valign);

        break;

    case ATTR_WEIGHT:
        widget->attributes.weight = attr_update(ATTR_WEIGHT, value, widget->attributes.weight);

        break;

    case ATTR_WRAP:
        widget->attributes.wrap = attr_update(ATTR_WRAP, value, widget->attributes.wrap);

        break;

    }

}

void widget_unsetattributes(struct widget *widget)
{

    widget_setattribute(widget, ATTR_COLOR, 0);
    widget_setattribute(widget, ATTR_CURSOR, 0);
    widget_setattribute(widget, ATTR_DISPLAY, 0);
    widget_setattribute(widget, ATTR_FLOW, 0);
    widget_setattribute(widget, ATTR_HALIGN, 0);
    widget_setattribute(widget, ATTR_ID, 0);
    widget_setattribute(widget, ATTR_IN, 0);
    widget_setattribute(widget, ATTR_LABEL, 0);
    widget_setattribute(widget, ATTR_MIMETYPE, 0);
    widget_setattribute(widget, ATTR_MODE, 0);
    widget_setattribute(widget, ATTR_ONCLICK, 0);
    widget_setattribute(widget, ATTR_OVERFLOW, 0);
    widget_setattribute(widget, ATTR_PADDING, 0);
    widget_setattribute(widget, ATTR_SOURCE, 0);
    widget_setattribute(widget, ATTR_SPAN, 0);
    widget_setattribute(widget, ATTR_VALIGN, 0);
    widget_setattribute(widget, ATTR_WEIGHT, 0);
    widget_setattribute(widget, ATTR_WRAP, 0);

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

unsigned int widget_region_intersectsx(struct widget *widget, int x)
{

    return util_intersects(x, widget->placement.position.x, widget->placement.position.x + widget->placement.size.w);

}

unsigned int widget_region_intersectsy(struct widget *widget, int y)
{

    return util_intersects(y, widget->placement.position.y, widget->placement.position.y + widget->placement.size.h);

}

unsigned int widget_region_intersects(struct widget *widget, int x, int y)
{

    return widget_region_intersectsx(widget, x) && widget_region_intersectsy(widget, y);

}

unsigned int widget_clip_intersectsx(struct widget *widget, int x)
{

    return util_intersects(x, widget->clip.position.x, widget->clip.position.x + widget->clip.size.w);

}

unsigned int widget_clip_intersectsy(struct widget *widget, int y)
{

    return util_intersects(y, widget->clip.position.y, widget->clip.position.y + widget->clip.size.h);

}

unsigned int widget_clip_intersects(struct widget *widget, int x, int y)
{

    return widget_clip_intersectsx(widget, x) && widget_clip_intersectsy(widget, y);

}

unsigned int widget_intersectsx(struct widget *widget, int x)
{

    return widget_region_intersectsx(widget, x) && widget_clip_intersectsx(widget, x);

}

unsigned int widget_intersectsy(struct widget *widget, int y)
{

    return widget_region_intersectsy(widget, y) && widget_clip_intersectsy(widget, y);

}

unsigned int widget_intersects(struct widget *widget, int x, int y)
{

    return widget_region_intersects(widget, x, y) && widget_clip_intersects(widget, x, y);

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

void widget_init_attributes(struct widget_attributes *attributes, unsigned type)
{

    attributes->color = 0;
    attributes->cursor = 0;
    attributes->display = ATTR_DISPLAY_BLOCK;
    attributes->flow = ATTR_FLOW_DEFAULT;
    attributes->halign = ATTR_HALIGN_LEFT;
    attributes->id = 0;
    attributes->in = 0;
    attributes->label = 0;
    attributes->mimetype = ATTR_MIMETYPE_NONE;
    attributes->mode = ATTR_MODE_NORMAL;
    attributes->onclick = 0;
    attributes->overflow = ATTR_OVERFLOW_NONE;
    attributes->padding = 0;
    attributes->span = 0;
    attributes->source = 0;
    attributes->valign = ATTR_VALIGN_TOP;
    attributes->weight = ATTR_WEIGHT_NORMAL;
    attributes->wrap = ATTR_WRAP_NONE;

    switch (type)
    {

    case WIDGET_TYPE_BUTTON:
        attributes->halign = ATTR_HALIGN_CENTER;
        attributes->valign = ATTR_VALIGN_MIDDLE;
        attributes->weight = ATTR_WEIGHT_BOLD;

        break;

    case WIDGET_TYPE_CHOICE:
        attributes->valign = ATTR_VALIGN_MIDDLE;

        break;

    case WIDGET_TYPE_SELECT:
        attributes->valign = ATTR_VALIGN_MIDDLE;

        break;

    case WIDGET_TYPE_TEXTBUTTON:
        attributes->valign = ATTR_VALIGN_MIDDLE;

        break;

    case WIDGET_TYPE_WINDOW:
        attributes->halign = ATTR_HALIGN_CENTER;
        attributes->valign = ATTR_VALIGN_MIDDLE;
        attributes->weight = ATTR_WEIGHT_BOLD;

        break;

    }

}

void widget_init(struct widget *widget, unsigned int source, unsigned int type)
{

    widget->source = source;
    widget->type = type;
    widget->state = WIDGET_STATE_NORMAL;
    widget->markstart = 0;
    widget->markend = 0;
    widget->enablecursor = 0;
    widget->resource = 0;
    widget->loaded = 0;
    widget->position = util_position(0, 0);
    widget->size = util_size(0, 0);
    widget->scroll = util_position(0, 0);
    widget->cursorposition = util_position(0, 0);
    widget->cursorsize = util_size(0, 0);
    widget->placement = util_region(0, 0, 0, 0);
    widget->clip = util_region(0, 0, 0, 0);
    widget->rowstart = util_position(0, 0);
    widget->rowstop = util_position(0, 0);

    widget_init_attributes(&widget->attributes, widget->type);

    switch (widget->type)
    {

    case WIDGET_TYPE_TEXTBOX:
        widget->enablecursor = 1;

        break;

    }

}

