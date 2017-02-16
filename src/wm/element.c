#include <abi.h>
#include <fudge.h>
#include "box.h"
#include "element.h"

void element_init(struct element *element, unsigned int id, unsigned int type, unsigned int z)
{

    element->id = id;
    element->type = type;
    element->z = z;

}

void element_initfill(struct element_fill *fill, unsigned int color)
{

    element_init(&fill->element, (unsigned int)fill, ELEMENT_TYPE_FILL, 1);

    fill->color = color;

}

void element_initmouse(struct element_mouse *mouse, unsigned int type)
{

    element_init(&mouse->element, (unsigned int)mouse, ELEMENT_TYPE_MOUSE, 2);

    mouse->type = type;

}

void element_initpanel(struct element_panel *panel, unsigned int active)
{

    element_init(&panel->element, (unsigned int)panel, ELEMENT_TYPE_PANEL, 1);

    panel->active = active;

}

void element_inittext(struct element_text *text, unsigned int type, unsigned int flow)
{

    element_init(&text->element, (unsigned int)text, ELEMENT_TYPE_TEXT, 1);

    text->type = type;
    text->flow = flow;
    text->cursor = 0;

}

void element_initwindow(struct element_window *window, unsigned int active)
{

    element_init(&window->element, (unsigned int)window, ELEMENT_TYPE_WINDOW, 1);

    window->active = active;

}

void element_set(struct element *element, unsigned int source, unsigned int damage, unsigned int count)
{

    element->source = source;
    element->damage = damage;
    element->count = count;

}

