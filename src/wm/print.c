#include <abi.h>
#include <fudge.h>
#include "box.h"
#include "element.h"

void print_insertfill(struct ring *ring, unsigned int source, struct element_fill *fill, unsigned int z)
{

    element_init(&fill->element, (unsigned int)fill, ELEMENT_TYPE_FILL, source, z, ELEMENT_DAMAGE_UPDATE, sizeof (struct element_fill));
    ring_write(ring, fill, sizeof (struct element_fill));

}

void print_insertmouse(struct ring *ring, unsigned int source, struct element_mouse *mouse, unsigned int z)
{

    element_init(&mouse->element, (unsigned int)mouse, ELEMENT_TYPE_MOUSE, source, z, ELEMENT_DAMAGE_UPDATE, sizeof (struct element_mouse));
    ring_write(ring, mouse, sizeof (struct element_mouse));

}

void print_insertpanel(struct ring *ring, unsigned int source, struct element_panel *panel, unsigned int z)
{

    element_init(&panel->element, (unsigned int)panel, ELEMENT_TYPE_PANEL, source, z, ELEMENT_DAMAGE_UPDATE, sizeof (struct element_panel));
    ring_write(ring, panel, sizeof (struct element_panel));

}

void print_inserttext(struct ring *ring, unsigned int source, struct element_text *text, unsigned int z, unsigned int count)
{

    element_init(&text->element, (unsigned int)text, ELEMENT_TYPE_TEXT, source, z, ELEMENT_DAMAGE_UPDATE, sizeof (struct element_text) + count);
    ring_write(ring, text, sizeof (struct element_text));

}

void print_insertwindow(struct ring *ring, unsigned int source, struct element_window *window, unsigned int z)
{

    element_init(&window->element, (unsigned int)window, ELEMENT_TYPE_WINDOW, source, z, ELEMENT_DAMAGE_UPDATE, sizeof (struct element_window));
    ring_write(ring, window, sizeof (struct element_window));

}

void print_removefill(struct ring *ring, unsigned int source, struct element_fill *fill, unsigned int z)
{

    element_init(&fill->element, (unsigned int)fill, ELEMENT_TYPE_FILL, source, z, ELEMENT_DAMAGE_REMOVE, sizeof (struct element_fill));
    ring_write(ring, fill, sizeof (struct element_fill));

}

void print_removemouse(struct ring *ring, unsigned int source, struct element_mouse *mouse, unsigned int z)
{

    element_init(&mouse->element, (unsigned int)mouse, ELEMENT_TYPE_MOUSE, source, z, ELEMENT_DAMAGE_REMOVE, sizeof (struct element_mouse));
    ring_write(ring, mouse, sizeof (struct element_mouse));

}

void print_removepanel(struct ring *ring, unsigned int source, struct element_panel *panel, unsigned int z)
{

    element_init(&panel->element, (unsigned int)panel, ELEMENT_TYPE_PANEL, source, z, ELEMENT_DAMAGE_REMOVE, sizeof (struct element_panel));
    ring_write(ring, panel, sizeof (struct element_panel));

}

void print_removetext(struct ring *ring, unsigned int source, struct element_text *text, unsigned int z)
{

    element_init(&text->element, (unsigned int)text, ELEMENT_TYPE_TEXT, source, z, ELEMENT_DAMAGE_REMOVE, sizeof (struct element_text));
    ring_write(ring, text, sizeof (struct element_text));

}

void print_removewindow(struct ring *ring, unsigned int source, struct element_window *window, unsigned int z)
{

    element_init(&window->element, (unsigned int)window, ELEMENT_TYPE_WINDOW, source, z, ELEMENT_DAMAGE_REMOVE, sizeof (struct element_window));
    ring_write(ring, window, sizeof (struct element_window));

}

