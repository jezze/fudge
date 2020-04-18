#include <fudge.h>
#include "resource.h"
#include "debug.h"

void debug_log(unsigned int level, char *string, char *file, unsigned int line)
{

    struct resource *current = 0;

    while ((current = resource_foreachtype(current, RESOURCE_DEBUGLOG)))
    {

        struct debug_interface *interface = current->data;

        interface->write(level, string, file, line);

    }

}

void debug_logs(unsigned int level, char *string)
{

    debug_log(level, string, 0, 0);

}

void debug_log8(unsigned int level, char *key, char value)
{

    char num[FUDGE_NSIZE];
    char string[FUDGE_BSIZE];
    unsigned int c = 0;

    c += memory_write(string, FUDGE_BSIZE, key, ascii_length(key), c);
    c += memory_write(string, FUDGE_BSIZE, ": ", 2, c);
    c += memory_write(string, FUDGE_BSIZE, num, ascii_wvalue(num, FUDGE_NSIZE, value, 2, 8), c);
    c += memory_write(string, FUDGE_BSIZE, " ", 1, c);
    c += memory_write(string, FUDGE_BSIZE, num, ascii_wvalue(num, FUDGE_NSIZE, value, 16, 2), c);
    c += memory_write(string, FUDGE_BSIZE, " ", 1, c);
    c += memory_write(string, FUDGE_BSIZE, num, ascii_wvalue(num, FUDGE_NSIZE, value, 10, 0), c);

    debug_log(level, string, 0, 0);

}

void debug_log16(unsigned int level, char *key, short value)
{

    char num[FUDGE_NSIZE];
    char string[FUDGE_BSIZE];
    unsigned int c = 0;

    c += memory_write(string, FUDGE_BSIZE, key, ascii_length(key), c);
    c += memory_write(string, FUDGE_BSIZE, ": ", 2, c);
    c += memory_write(string, FUDGE_BSIZE, num, ascii_wvalue(num, FUDGE_NSIZE, value, 2, 16), c);
    c += memory_write(string, FUDGE_BSIZE, " ", 1, c);
    c += memory_write(string, FUDGE_BSIZE, num, ascii_wvalue(num, FUDGE_NSIZE, value, 16, 4), c);
    c += memory_write(string, FUDGE_BSIZE, " ", 1, c);
    c += memory_write(string, FUDGE_BSIZE, num, ascii_wvalue(num, FUDGE_NSIZE, value, 10, 0), c);

    debug_log(level, string, 0, 0);

}

void debug_log32(unsigned int level, char *key, int value)
{

    char num[FUDGE_NSIZE];
    char string[FUDGE_BSIZE];
    unsigned int c = 0;

    c += memory_write(string, FUDGE_BSIZE, key, ascii_length(key), c);
    c += memory_write(string, FUDGE_BSIZE, ": ", 2, c);
    c += memory_write(string, FUDGE_BSIZE, num, ascii_wvalue(num, FUDGE_NSIZE, value, 2, 32), c);
    c += memory_write(string, FUDGE_BSIZE, " ", 1, c);
    c += memory_write(string, FUDGE_BSIZE, num, ascii_wvalue(num, FUDGE_NSIZE, value, 16, 8), c);
    c += memory_write(string, FUDGE_BSIZE, " ", 1, c);
    c += memory_write(string, FUDGE_BSIZE, num, ascii_wvalue(num, FUDGE_NSIZE, value, 10, 0), c);

    debug_log(level, string, 0, 0);

}

void debug_assert(unsigned int level, unsigned int test, char *file, unsigned int line)
{

    if (!test)
        debug_log(level, "ASSERT FAIL", file, line);

}

void debug_initinterface(struct debug_interface *interface, void (*write)(unsigned int level, char *string, char *file, unsigned int line))
{

    resource_init(&interface->resource, RESOURCE_DEBUGLOG, interface);

    interface->write = write;

}

