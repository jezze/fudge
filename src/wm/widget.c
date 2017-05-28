#include <abi.h>
#include <fudge.h>
#include "box.h"
#include "widget.h"

void widget_init(struct widget *widget, unsigned int id, unsigned int type, unsigned int z)
{

    widget->id = id;
    widget->type = type;
    widget->z = z;

}

void widget_initfill(struct widget_fill *fill, unsigned int color)
{

    widget_init(&fill->widget, (unsigned int)fill, WIDGET_TYPE_FILL, 1);

    fill->color = color;

}

void widget_initmouse(struct widget_mouse *mouse, unsigned int type)
{

    widget_init(&mouse->widget, (unsigned int)mouse, WIDGET_TYPE_MOUSE, 2);

    mouse->type = type;

}

void widget_initpanel(struct widget_panel *panel, unsigned int active)
{

    widget_init(&panel->widget, (unsigned int)panel, WIDGET_TYPE_PANEL, 1);

    panel->active = active;

}

void widget_inittext(struct widget_text *text, unsigned int type, unsigned int flow)
{

    widget_init(&text->widget, (unsigned int)text, WIDGET_TYPE_TEXT, 1);

    text->type = type;
    text->flow = flow;
    text->cursor = 0;

}

void widget_initwindow(struct widget_window *window, unsigned int active)
{

    widget_init(&window->widget, (unsigned int)window, WIDGET_TYPE_WINDOW, 1);

    window->active = active;

}

void widget_set(struct widget *widget, unsigned int source, unsigned int damage, unsigned int count)
{

    widget->source = source;
    widget->damage = damage;
    widget->count = count;

}

