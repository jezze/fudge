#include <abi.h>
#include <fudge.h>
#include "box.h"
#include "element.h"

static void printelement(struct buffer *buffer, unsigned int id, unsigned int func, unsigned int type, unsigned int source, unsigned int z, unsigned int count)
{

    struct element element;

    element_init(&element, id, func, type, source, z, count);
    buffer_write(buffer, &element, sizeof (struct element));

}

void print_insertfill(struct buffer *buffer, unsigned int source, struct element_fill *fill, unsigned int z)
{

    printelement(buffer, (unsigned int)fill, ELEMENT_FUNC_INSERT, ELEMENT_TYPE_FILL, source, z, sizeof (struct element_fill));
    buffer_write(buffer, fill, sizeof (struct element_fill));

}

void print_insertmouse(struct buffer *buffer, unsigned int source, struct element_mouse *mouse, unsigned int z)
{

    printelement(buffer, (unsigned int)mouse, ELEMENT_FUNC_INSERT, ELEMENT_TYPE_MOUSE, source, z, sizeof (struct element_mouse));
    buffer_write(buffer, mouse, sizeof (struct element_mouse));

}

void print_insertpanel(struct buffer *buffer, unsigned int source, struct element_panel *panel, unsigned int z)
{

    printelement(buffer, (unsigned int)panel, ELEMENT_FUNC_INSERT, ELEMENT_TYPE_PANEL, source, z, sizeof (struct element_panel));
    buffer_write(buffer, panel, sizeof (struct element_panel));

}

void print_inserttext(struct buffer *buffer, unsigned int source, struct element_text *text, unsigned int z, void *textbuffer, unsigned int count)
{

    printelement(buffer, (unsigned int)text, ELEMENT_FUNC_INSERT, ELEMENT_TYPE_TEXT, source, z, sizeof (struct element_text) + count);
    buffer_write(buffer, text, sizeof (struct element_text));
    buffer_write(buffer, textbuffer, count);

}

void print_insertwindow(struct buffer *buffer, unsigned int source, struct element_window *window, unsigned int z)
{

    printelement(buffer, (unsigned int)window, ELEMENT_FUNC_INSERT, ELEMENT_TYPE_WINDOW, source, z, sizeof (struct element_window));
    buffer_write(buffer, window, sizeof (struct element_window));

}

void print_removefill(struct buffer *buffer, unsigned int source, struct element_fill *fill)
{

    printelement(buffer, (unsigned int)fill, ELEMENT_FUNC_REMOVE, ELEMENT_TYPE_FILL, source, 0, 0);

}

void print_removemouse(struct buffer *buffer, unsigned int source, struct element_mouse *mouse)
{

    printelement(buffer, (unsigned int)mouse, ELEMENT_FUNC_REMOVE, ELEMENT_TYPE_MOUSE, source, 0, 0);

}

void print_removepanel(struct buffer *buffer, unsigned int source, struct element_panel *panel)
{

    printelement(buffer, (unsigned int)panel, ELEMENT_FUNC_REMOVE, ELEMENT_TYPE_PANEL, source, 0, 0);

}

void print_removetext(struct buffer *buffer, unsigned int source, struct element_text *text)
{

    printelement(buffer, (unsigned int)text, ELEMENT_FUNC_REMOVE, ELEMENT_TYPE_TEXT, source, 0, 0);

}

void print_removewindow(struct buffer *buffer, unsigned int source, struct element_window *window)
{

    printelement(buffer, (unsigned int)window, ELEMENT_FUNC_REMOVE, ELEMENT_TYPE_WINDOW, source, 0, 0);

}

