#include <abi.h>
#include <fudge.h>
#include "box.h"
#include "element.h"

void element_init(struct element *element, unsigned int id, unsigned int type, unsigned int source, unsigned int z, unsigned int count)
{

    element->id = id;
    element->type = type;
    element->source = source;
    element->z = z;
    element->damaged = 0;
    element->count = count;

}

void element_initfill(struct element_fill *fill, unsigned int color)
{

    box_setsize(&fill->size, 0, 0, 0, 0);

    fill->color = color;

}

void element_initmouse(struct element_mouse *mouse)
{

}

void element_initpanel(struct element_panel *panel)
{

    box_setsize(&panel->size, 0, 0, 0, 0);

}

void element_inittext(struct element_text *text, unsigned int type, unsigned int rows)
{

    box_setsize(&text->size, 0, 0, 0, 0);

    text->type = type;
    text->rows = rows;

}

void element_initwindow(struct element_window *window)
{

    box_setsize(&window->size, 0, 0, 0, 0);

}

