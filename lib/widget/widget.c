#include <fudge.h>
#include <abi.h>
#include "box.h"
#include "widget.h"

void widget_initfill(struct widget_fill *fill, unsigned int color)
{

    fill->color = color;

}

void widget_initmouse(struct widget_mouse *mouse, unsigned int type)
{

    mouse->type = type;

}

void widget_initpanel(struct widget_panel *panel, unsigned int active)
{

    panel->active = active;

}

void widget_inittext(struct widget_text *text, unsigned int type)
{

    text->type = type;

}

void widget_inittextbox(struct widget_textbox *textbox)
{

}

void widget_initwindow(struct widget_window *window, unsigned int active)
{

    window->active = active;

}

void widget_update(struct ring *ring, void *item, unsigned int z, unsigned int source, unsigned int type, unsigned int count, unsigned int x, unsigned int y, unsigned int w, unsigned int h)
{

    struct widget widget;

    widget.id = (unsigned int)item;
    widget.type = type;
    widget.z = z;
    widget.damage = WIDGET_DAMAGE_REPLACE;
    widget.source = source;
    widget.count = sizeof (struct widget) + count;
    widget.size.x = x;
    widget.size.y = y;
    widget.size.w = w;
    widget.size.h = h;

    ring_write(ring, &widget, sizeof (struct widget));

}

void widget_remove(struct ring *ring, void *item, unsigned int z, unsigned int source)
{

    struct widget widget;

    widget.id = (unsigned int)item;
    widget.type = WIDGET_TYPE_NULL;
    widget.z = z;
    widget.damage = WIDGET_DAMAGE_REMOVE;
    widget.source = source;
    widget.count = sizeof (struct widget);
    widget.size.x = 0;
    widget.size.y = 0;
    widget.size.w = 0;
    widget.size.h = 0;

    ring_write(ring, &widget, sizeof (struct widget));

}

