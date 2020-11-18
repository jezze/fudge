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

void debug_log8(unsigned int level, char *key, unsigned char value)
{

    char num[ASCII_NUMSIZE];
    char string[BUFFER_SIZE];
    unsigned int c = 0;

    c += buffer_write(string, BUFFER_SIZE, key, ascii_length(key), c);
    c += buffer_write(string, BUFFER_SIZE, ": ", 2, c);
    c += buffer_write(string, BUFFER_SIZE, num, ascii_wvalue(num, ASCII_NUMSIZE, value, 2, 8), c);
    c += buffer_write(string, BUFFER_SIZE, " ", 1, c);
    c += buffer_write(string, BUFFER_SIZE, num, ascii_wvalue(num, ASCII_NUMSIZE, value, 16, 2), c);
    c += buffer_write(string, BUFFER_SIZE, " ", 1, c);
    c += buffer_write(string, BUFFER_SIZE, num, ascii_wvalue(num, ASCII_NUMSIZE, value, 10, 0), c);
    c += buffer_write(string, BUFFER_SIZE, "", 1, c);

    debug_log(level, string, 0, 0);

}

void debug_log16(unsigned int level, char *key, unsigned short value)
{

    char num[ASCII_NUMSIZE];
    char string[BUFFER_SIZE];
    unsigned int c = 0;

    c += buffer_write(string, BUFFER_SIZE, key, ascii_length(key), c);
    c += buffer_write(string, BUFFER_SIZE, ": ", 2, c);
    c += buffer_write(string, BUFFER_SIZE, num, ascii_wvalue(num, ASCII_NUMSIZE, value, 2, 16), c);
    c += buffer_write(string, BUFFER_SIZE, " ", 1, c);
    c += buffer_write(string, BUFFER_SIZE, num, ascii_wvalue(num, ASCII_NUMSIZE, value, 16, 4), c);
    c += buffer_write(string, BUFFER_SIZE, " ", 1, c);
    c += buffer_write(string, BUFFER_SIZE, num, ascii_wvalue(num, ASCII_NUMSIZE, value, 10, 0), c);
    c += buffer_write(string, BUFFER_SIZE, "", 1, c);

    debug_log(level, string, 0, 0);

}

void debug_log32(unsigned int level, char *key, unsigned int value)
{

    char num[ASCII_NUMSIZE];
    char string[BUFFER_SIZE];
    unsigned int c = 0;

    c += buffer_write(string, BUFFER_SIZE, key, ascii_length(key), c);
    c += buffer_write(string, BUFFER_SIZE, ": ", 2, c);
    c += buffer_write(string, BUFFER_SIZE, num, ascii_wvalue(num, ASCII_NUMSIZE, value, 2, 32), c);
    c += buffer_write(string, BUFFER_SIZE, " ", 1, c);
    c += buffer_write(string, BUFFER_SIZE, num, ascii_wvalue(num, ASCII_NUMSIZE, value, 16, 8), c);
    c += buffer_write(string, BUFFER_SIZE, " ", 1, c);
    c += buffer_write(string, BUFFER_SIZE, num, ascii_wvalue(num, ASCII_NUMSIZE, value, 10, 0), c);
    c += buffer_write(string, BUFFER_SIZE, "", 1, c);

    debug_log(level, string, 0, 0);

}

void debug_assert(unsigned int level, unsigned int test, char *file, unsigned int line)
{

    if (!test)
        debug_log(level, "ASSERT FAIL", file, line);

}

void debug_registerinterface(struct debug_interface *interface)
{

    resource_register(&interface->resource);

}

void debug_unregisterinterface(struct debug_interface *interface)
{

    resource_unregister(&interface->resource);

}

void debug_initinterface(struct debug_interface *interface, void (*write)(unsigned int level, char *string, char *file, unsigned int line))
{

    resource_init(&interface->resource, RESOURCE_DEBUGLOG, interface);

    interface->write = write;

}

