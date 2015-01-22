#include <fudge.h>
#include <kernel.h>
#include <system/system.h>
#include <base/base.h>
#include "log.h"

static unsigned char databuffer[4096];
static struct buffer data;
static struct system_node messages;

static unsigned int messages_read(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    return buffer_rcfifo(&data, count, databuffer);

}

static unsigned int messages_write(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    return buffer_wcfifo(&data, count, databuffer);

}

unsigned int log_write(unsigned int count, char *buffer)
{

    return buffer_wcfifo(&data, count, databuffer);

}

void init()
{

    buffer_init(&data, 1, 4096, databuffer);
    system_initnode(&messages, SYSTEM_NODETYPE_NORMAL, "messages");
    system_registernode(&messages);

    messages.read = messages_read;
    messages.write = messages_write;

}

void destroy()
{

    system_unregisternode(&messages);

}

