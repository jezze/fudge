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
    element->count = count;

}

void element_initmouse(struct element_mouse *mouse)
{

    box_setsize(&mouse->size, 0, 0, 0, 0);

}

void element_initpanel(struct element_panel *panel)
{

    box_setsize(&panel->size, 0, 0, 0, 0);

}

void element_inittext(struct element_text *text, unsigned int type)
{

    box_setsize(&text->size, 0, 0, 0, 0);

    text->type = type;

}

void element_initwindow(struct element_window *window)
{

    box_setsize(&window->size, 0, 0, 0, 0);

}

