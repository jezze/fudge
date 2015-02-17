#include <fudge.h>
#include <kernel.h>
#include <system/system.h>
#include "log.h"

static struct system_node messages;

static unsigned int messages_read(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    return 0;

}

static unsigned int messages_write(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    return 0;

}

unsigned int log_write(unsigned int count, char *buffer)
{

    return 0;

}

void module_init()
{

    system_initnode(&messages, SYSTEM_NODETYPE_NORMAL, "messages");

    messages.read = messages_read;
    messages.write = messages_write;

}

void module_register()
{

    system_registernode(&messages);

}

void module_unregister()
{

    system_unregisternode(&messages);

}

