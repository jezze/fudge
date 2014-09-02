#include <module.h>
#include <kernel/resource.h>
#include <kernel/vfs.h>
#include <system/system.h>
#include <base/base.h>
#include "log.h"

static char dbuffer[0x1000];
static unsigned int doffset;
static struct system_node messages;

static unsigned int messages_read(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    return memory_read(buffer, count, dbuffer, 0x1000, offset);

}

unsigned int log_write(unsigned int count, char *buffer)
{

    count = memory_write(dbuffer, 0x1000, buffer, count, doffset);
    doffset += count;

    return count;

}

unsigned int log_writenumber(unsigned int number, unsigned int base)
{

    char n[32];
    unsigned int count = ascii_fromint(n, 32, number, base);

    return log_write(count, n);

}

unsigned int log_writestring(char *buffer)
{

    return log_write(ascii_length(buffer), buffer);

}

void init()
{

    doffset = 0;

    system_initstream(&messages, "messages");
    system_registernode(&messages);

    messages.read = messages_read;

}

void destroy()
{

    system_unregisternode(&messages);

}

