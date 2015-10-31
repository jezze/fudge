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

}

void element_initpanel(struct element_panel *panel)
{

}

void element_inittext(struct element_text *text, unsigned int type)
{

    text->type = type;

}

void element_initwindow(struct element_window *window)
{

}

