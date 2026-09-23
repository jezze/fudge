#include <fudge.h>
#include <kernel.h>
#include "console.h"

static struct node_operands operands;

static unsigned int ondata(struct console_interface *interface, unsigned int source, void *data, unsigned int count)
{

    if (interface->ondata)
    {

        interface->ondata(data, count);

        return MESSAGE_OK;

    }

    return MESSAGE_FAILED;

}

static unsigned int operands_place(struct resource *resource, unsigned int source, unsigned int event, unsigned int count, void *data)
{

    struct console_interface *interface = resource->data;

    switch (event)
    {

    case EVENT_LINK:
        return kernel_linknode(&interface->service, source);

    case EVENT_UNLINK:
        return kernel_unlinknode(&interface->service, source);

    case EVENT_DATA:
        return ondata(interface, source, data, count);

    }

    return MESSAGE_FAILED;

}

void console_notifydata(struct console_interface *interface, unsigned char data)
{

    struct event_consoledata consoledata;

    consoledata.data = data;

    kernel_notify(&interface->service, interface->inode, EVENT_CONSOLEDATA, sizeof (struct event_consoledata), &consoledata);

}

void console_cursorset(struct console_interface *interface, unsigned int position)
{

    interface->cursor = position;

}

void console_cursorleft(struct console_interface *interface, unsigned int steps)
{

    interface->cursor -= steps;

}

void console_cursorright(struct console_interface *interface, unsigned int steps)
{

    interface->cursor += steps;

}

void console_cursorup(struct console_interface *interface, unsigned int steps)
{

    interface->cursor -= interface->width * steps;

}

void console_cursordown(struct console_interface *interface, unsigned int steps)
{

    interface->cursor += interface->width * steps;

}

void console_cursorhome(struct console_interface *interface)
{

    interface->cursor -= (interface->cursor % interface->width);

}

void console_cursorend(struct console_interface *interface)
{

    interface->cursor += interface->width - (interface->cursor % interface->width) - 1;

}

void console_registerinterface(struct console_interface *interface)
{

    resource_register(&interface->resource);
    service_register(&interface->service, interface->inode);

}

void console_unregisterinterface(struct console_interface *interface)
{

    resource_unregister(&interface->resource);
    service_unregister(&interface->service);

}

void console_initinterface(struct console_interface *interface, unsigned int id, unsigned int (*ondata)(void *buffer, unsigned int count))
{

    resource_init(&interface->resource, RESOURCE_CONSOLEINTERFACE, interface);
    service_init(&interface->service, "console");

    interface->id = id;
    interface->width = 0;
    interface->height = 0;
    interface->color = 0;
    interface->cursor = 0;
    interface->inode = pool_picknode();
    interface->ondata = ondata;

    if (interface->inode)
    {

        struct node *node = pool_getnode(interface->inode);

        node_reset(node, &interface->resource, &operands);

    }

}

void module_init(void)
{

    node_operands_init(&operands, 0, operands_place);

}

