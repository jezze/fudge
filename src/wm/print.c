#include <abi.h>
#include <fudge.h>
#include "box.h"
#include "element.h"

static void printelement(struct buffer *buffer, unsigned int id, unsigned int type, unsigned int source, unsigned int z, unsigned int count)
{

    struct element element;

    element_init(&element, id, type, source, z, count);
    buffer_write(buffer, &element, sizeof (struct element));

}

void print_fill(struct buffer *buffer, unsigned int source, unsigned int z, struct element_fill *fill)
{

    printelement(buffer, (unsigned int)fill, ELEMENT_TYPE_FILL, source, z, sizeof (struct element_fill));
    buffer_write(buffer, fill, sizeof (struct element_fill));

}

void print_mouse(struct buffer *buffer, unsigned int source, unsigned int z, struct element_mouse *mouse)
{

    printelement(buffer, (unsigned int)mouse, ELEMENT_TYPE_MOUSE, source, z, sizeof (struct element_mouse));
    buffer_write(buffer, mouse, sizeof (struct element_mouse));

}

void print_panel(struct buffer *buffer, unsigned int source, unsigned int z, struct element_panel *panel)
{

    printelement(buffer, (unsigned int)panel, ELEMENT_TYPE_PANEL, source, z, sizeof (struct element_panel));
    buffer_write(buffer, panel, sizeof (struct element_panel));

}

void print_text(struct buffer *buffer, unsigned int source, unsigned int z, struct element_text *text, void *textbuffer, unsigned int count)
{

    printelement(buffer, (unsigned int)text, ELEMENT_TYPE_TEXT, source, z, sizeof (struct element_text) + count);
    buffer_write(buffer, text, sizeof (struct element_text));
    buffer_write(buffer, textbuffer, count);

}

void print_window(struct buffer *buffer, unsigned int source, unsigned int z, struct element_window *window)
{

    printelement(buffer, (unsigned int)window, ELEMENT_TYPE_WINDOW, source, z, sizeof (struct element_window));
    buffer_write(buffer, window, sizeof (struct element_window));

}

