#include <fudge/module.h>
#include <base/base.h>
#include <system/system.h>
#include "log.h"

static char dbuffer[0x1000];
static unsigned int doffset;
static struct system_stream messages;

static unsigned int messages_read(struct system_stream *self, unsigned int offset, unsigned int count, void *buffer)
{

    return memory_read(buffer, count, dbuffer, 0x1000, offset);

}

static unsigned int messages_write(struct system_stream *self, unsigned int offset, unsigned int count, void *buffer)
{

    return 0;

}

unsigned int log_write(unsigned int count, char *buffer)
{

    count = memory_write(dbuffer, 0x1000, buffer, count, doffset);
    doffset += count;

    return count;

}

unsigned int log_write_number(unsigned int number, unsigned int base)
{

    char n[32];
    unsigned int count = memory_write_number(n, 32, number, base, 0);

    return log_write(count, n);

}

void init()
{

    doffset = 0;

    system_init_stream(&messages, "messages", messages_read, messages_write);
    system_register_node(&messages.node);

}

void destroy()
{

    system_unregister_node(&messages.node);

}

