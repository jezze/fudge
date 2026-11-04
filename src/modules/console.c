#include <fudge.h>
#include <kernel.h>
#include "console.h"

static struct resource_operands operands;
static struct unitsrv unitsrv;
static struct service service;

static unsigned int ondata(struct console_interface *interface, unsigned int source, void *data, unsigned int count)
{

    if (interface->send)
    {

        interface->send(data, count);

        return MESSAGE_OK;

    }

    return MESSAGE_FAILED;

}

static unsigned int operands_place(unsigned int target, unsigned int source, unsigned int event, unsigned int count, void *data)
{

    struct resource *resource = pool_getnoderesource(target);

    if (resource)
    {

        struct console_interface *interface = resource->data;

        switch (event)
        {

        case MESSAGE_TYPE_LINK:
            return kernel_linknode(&interface->resource, source);

        case MESSAGE_TYPE_UNLINK:
            return kernel_unlinknode(&interface->resource, source);

        case MESSAGE_TYPE_DATA:
            return ondata(interface, source, data, count);

        case MESSAGE_TYPE_ERROR:
            ondata(interface, source, "[ERROR] ", 8);

            return ondata(interface, source, data, count);

        }

    }

    return MESSAGE_FAILED;

}

void console_notifydata(struct console_interface *interface, unsigned char data)
{

    kernel_notify(&interface->resource, MESSAGE_TYPE_CONSOLEDATA, 1, &data);

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

}

void console_unregisterinterface(struct console_interface *interface)
{

    resource_unregister(&interface->resource);

}

void console_initinterface(struct console_interface *interface, unsigned int (*send)(void *buffer, unsigned int count))
{

    resource_init(&interface->resource, RESOURCE_CONSOLEINTERFACE, interface, &operands);

    interface->width = 0;
    interface->height = 0;
    interface->color = 0;
    interface->cursor = 0;
    interface->send = send;

}

void module_init(void)
{

    resource_operands_init(&operands, 0, operands_place, 0);

    unitsrv_init(&unitsrv, RESOURCE_CONSOLEINTERFACE);
    service_init(&service, "console");

}

void module_register(void)
{

    service_register(&service, pool_attachnode(&unitsrv.server.resource));

}

void module_unregister(void)
{

    service_unregister(&service);
    pool_detachnode(&unitsrv.server.resource);

}

