#include <abi.h>
#include <fudge.h>
#include "box.h"
#include "widget.h"

void print_insertfill(struct ring *ring, struct event_header *header, struct widget_fill *fill)
{

    widget_set(&fill->widget, header->destination, WIDGET_DAMAGE_UPDATE, sizeof (struct widget_fill));
    ring_write(ring, fill, sizeof (struct widget_fill));

}

void print_insertmouse(struct ring *ring, struct event_header *header, struct widget_mouse *mouse)
{

    widget_set(&mouse->widget, header->destination, WIDGET_DAMAGE_UPDATE, sizeof (struct widget_mouse));
    ring_write(ring, mouse, sizeof (struct widget_mouse));

}

void print_insertpanel(struct ring *ring, struct event_header *header, struct widget_panel *panel)
{

    widget_set(&panel->widget, header->destination, WIDGET_DAMAGE_UPDATE, sizeof (struct widget_panel));
    ring_write(ring, panel, sizeof (struct widget_panel));

}

void print_inserttext(struct ring *ring, struct event_header *header, struct widget_text *text, unsigned int count)
{

    widget_set(&text->widget, header->destination, WIDGET_DAMAGE_UPDATE, sizeof (struct widget_text) + count);
    ring_write(ring, text, sizeof (struct widget_text));

}

void print_insertwindow(struct ring *ring, struct event_header *header, struct widget_window *window)
{

    widget_set(&window->widget, header->destination, WIDGET_DAMAGE_UPDATE, sizeof (struct widget_window));
    ring_write(ring, window, sizeof (struct widget_window));

}

void print_removefill(struct ring *ring, struct event_header *header, struct widget_fill *fill)
{

    widget_set(&fill->widget, header->destination, WIDGET_DAMAGE_REMOVE, sizeof (struct widget_fill));
    ring_write(ring, fill, sizeof (struct widget_fill));

}

void print_removemouse(struct ring *ring, struct event_header *header, struct widget_mouse *mouse)
{

    widget_set(&mouse->widget, header->destination, WIDGET_DAMAGE_REMOVE, sizeof (struct widget_mouse));
    ring_write(ring, mouse, sizeof (struct widget_mouse));

}

void print_removepanel(struct ring *ring, struct event_header *header, struct widget_panel *panel)
{

    widget_set(&panel->widget, header->destination, WIDGET_DAMAGE_REMOVE, sizeof (struct widget_panel));
    ring_write(ring, panel, sizeof (struct widget_panel));

}

void print_removetext(struct ring *ring, struct event_header *header, struct widget_text *text)
{

    widget_set(&text->widget, header->destination, WIDGET_DAMAGE_REMOVE, sizeof (struct widget_text));
    ring_write(ring, text, sizeof (struct widget_text));

}

void print_removewindow(struct ring *ring, struct event_header *header, struct widget_window *window)
{

    widget_set(&window->widget, header->destination, WIDGET_DAMAGE_REMOVE, sizeof (struct widget_window));
    ring_write(ring, window, sizeof (struct widget_window));

}

