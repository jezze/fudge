#include <fudge.h>
#include "box.h"
#include "widget.h"

void widget_initfill(struct widget_fill *fill, unsigned int color)
{

    box_setsize(&fill->size, 0, 0, 0, 0);

    fill->color = color;

}

void widget_initmouse(struct widget_mouse *mouse, unsigned int type)
{

    box_setsize(&mouse->size, 0, 0, 0, 0);

    mouse->type = type;

}

void widget_initpanel(struct widget_panel *panel, unsigned int active)
{

    box_setsize(&panel->size, 0, 0, 0, 0);
    box_setsize(&panel->textbox, 0, 0, 0, 0);

    panel->active = active;
    panel->length = 0;

}

void widget_inittextbox(struct widget_textbox *textbox)
{

    box_setsize(&textbox->size, 0, 0, 0, 0);

    textbox->length = 0;
    textbox->cursor = 0;
    textbox->offset = 0;
    textbox->scroll = 0;

}

void widget_initwindow(struct widget_window *window, unsigned int active)
{

    box_setsize(&window->size, 0, 0, 0, 0);

    window->active = active;

}

void widget_update(struct ring *ring, void *item, unsigned int z, unsigned int type, unsigned int count)
{

    struct widget widget;

    widget.id = (unsigned int)item;
    widget.type = type;
    widget.z = z;
    widget.damage = WIDGET_DAMAGE_REPLACE;
    widget.count = sizeof (struct widget) + count;

    ring_write(ring, &widget, sizeof (struct widget));

}

void widget_remove(struct ring *ring, void *item, unsigned int z)
{

    struct widget widget;

    widget.id = (unsigned int)item;
    widget.type = WIDGET_TYPE_NULL;
    widget.z = z;
    widget.damage = WIDGET_DAMAGE_REMOVE;
    widget.count = sizeof (struct widget);

    ring_write(ring, &widget, sizeof (struct widget));

}

