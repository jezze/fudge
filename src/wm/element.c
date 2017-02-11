#include <abi.h>
#include <fudge.h>
#include "box.h"
#include "element.h"

void element_init(struct element *element, unsigned int id, unsigned int type, unsigned int source, unsigned int z, unsigned int damage, unsigned int count)
{

    element->id = id;
    element->type = type;
    element->source = source;
    element->z = z;
    element->damage = damage;
    element->count = count;

}

void element_initfill(struct element_fill *fill, unsigned int color)
{

    box_setsize(&fill->size, 0, 0, 0, 0);

    fill->color = color;

}

void element_initmouse(struct element_mouse *mouse, unsigned int x, unsigned int y)
{

    mouse->x = x;
    mouse->y = y;

}

void element_initpanel(struct element_panel *panel, unsigned int active)
{

    box_setsize(&panel->size, 0, 0, 0, 0);

    panel->active = active;

}

void element_inittext(struct element_text *text, unsigned int type, unsigned int flow)
{

    box_setsize(&text->size, 0, 0, 0, 0);

    text->type = type;
    text->flow = flow;
    text->cursor = 0;

}

void element_initwindow(struct element_window *window, unsigned int active)
{

    box_setsize(&window->size, 0, 0, 0, 0);

    window->active = active;

}

