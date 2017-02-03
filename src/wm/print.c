#include <abi.h>
#include <fudge.h>
#include "box.h"
#include "element.h"

static void printelement(struct ring *ring, unsigned int id, unsigned int func, unsigned int type, unsigned int source, unsigned int z, unsigned int count)
{

    struct element element;

    element_init(&element, id, func, type, source, z, count);
    ring_write(ring, &element, sizeof (struct element));

}

void print_insertconfig(struct ring *ring, unsigned int source, struct element_config *config, unsigned int z)
{

    printelement(ring, (unsigned int)config, ELEMENT_FUNC_INSERT, ELEMENT_TYPE_CONFIG, source, z, sizeof (struct element_config));
    ring_write(ring, config, sizeof (struct element_config));

}

void print_insertfill(struct ring *ring, unsigned int source, struct element_fill *fill, unsigned int z)
{

    printelement(ring, (unsigned int)fill, ELEMENT_FUNC_INSERT, ELEMENT_TYPE_FILL, source, z, sizeof (struct element_fill));
    ring_write(ring, fill, sizeof (struct element_fill));

}

void print_insertmouse(struct ring *ring, unsigned int source, struct element_mouse *mouse, unsigned int z)
{

    printelement(ring, (unsigned int)mouse, ELEMENT_FUNC_INSERT, ELEMENT_TYPE_MOUSE, source, z, sizeof (struct element_mouse));
    ring_write(ring, mouse, sizeof (struct element_mouse));

}

void print_insertpanel(struct ring *ring, unsigned int source, struct element_panel *panel, unsigned int z)
{

    printelement(ring, (unsigned int)panel, ELEMENT_FUNC_INSERT, ELEMENT_TYPE_PANEL, source, z, sizeof (struct element_panel));
    ring_write(ring, panel, sizeof (struct element_panel));

}

void print_inserttext(struct ring *ring, unsigned int source, struct element_text *text, unsigned int z, unsigned int count)
{

    printelement(ring, (unsigned int)text, ELEMENT_FUNC_INSERT, ELEMENT_TYPE_TEXT, source, z, sizeof (struct element_text) + count);
    ring_write(ring, text, sizeof (struct element_text));

}

void print_insertwindow(struct ring *ring, unsigned int source, struct element_window *window, unsigned int z)
{

    printelement(ring, (unsigned int)window, ELEMENT_FUNC_INSERT, ELEMENT_TYPE_WINDOW, source, z, sizeof (struct element_window));
    ring_write(ring, window, sizeof (struct element_window));

}

void print_removeconfig(struct ring *ring, unsigned int source, struct element_config *config)
{

    printelement(ring, (unsigned int)config, ELEMENT_FUNC_REMOVE, ELEMENT_TYPE_CONFIG, source, 0, 0);

}

void print_removefill(struct ring *ring, unsigned int source, struct element_fill *fill)
{

    printelement(ring, (unsigned int)fill, ELEMENT_FUNC_REMOVE, ELEMENT_TYPE_FILL, source, 0, 0);

}

void print_removemouse(struct ring *ring, unsigned int source, struct element_mouse *mouse)
{

    printelement(ring, (unsigned int)mouse, ELEMENT_FUNC_REMOVE, ELEMENT_TYPE_MOUSE, source, 0, 0);

}

void print_removepanel(struct ring *ring, unsigned int source, struct element_panel *panel)
{

    printelement(ring, (unsigned int)panel, ELEMENT_FUNC_REMOVE, ELEMENT_TYPE_PANEL, source, 0, 0);

}

void print_removetext(struct ring *ring, unsigned int source, struct element_text *text)
{

    printelement(ring, (unsigned int)text, ELEMENT_FUNC_REMOVE, ELEMENT_TYPE_TEXT, source, 0, 0);

}

void print_removewindow(struct ring *ring, unsigned int source, struct element_window *window)
{

    printelement(ring, (unsigned int)window, ELEMENT_FUNC_REMOVE, ELEMENT_TYPE_WINDOW, source, 0, 0);

}

