#include <fudge.h>
#include "resource.h"
#include "debug.h"

static void debug(unsigned int level, unsigned int count, char *string, char *file, unsigned int line)
{

    struct resource *current = 0;

    while ((current = resource_foreachtype(current, RESOURCE_DEBUGLOG)))
    {

        struct debug_interface *interface = current->data;

        interface->write(level, count, string, file, line);

    }

}

void debug_fmt0(unsigned int level, char *fmt, char *file, unsigned int line)
{

    char buffer[64];

    debug(level, cstring_write_fmt0(buffer, 64, fmt, 0), buffer, file, line); 

}

void debug_fmt1(unsigned int level, char *fmt, void *arg1, char *file, unsigned int line)
{

    char buffer[64];

    debug(level, cstring_write_fmt1(buffer, 64, fmt, 0, arg1), buffer, file, line); 

}

void debug_fmt2(unsigned int level, char *fmt, void *arg1, void *arg2, char *file, unsigned int line)
{

    char buffer[64];

    debug(level, cstring_write_fmt2(buffer, 64, fmt, 0, arg1, arg2), buffer, file, line); 

}

void debug_fmt3(unsigned int level, char *fmt, void *arg1, void *arg2, void *arg3, char *file, unsigned int line)
{

    char buffer[64];

    debug(level, cstring_write_fmt3(buffer, 64, fmt, 0, arg1, arg2, arg3), buffer, file, line); 

}

void debug_fmt4(unsigned int level, char *fmt, void *arg1, void *arg2, void *arg3, void *arg4, char *file, unsigned int line)
{

    char buffer[64];

    debug(level, cstring_write_fmt4(buffer, 64, fmt, 0, arg1, arg2, arg3, arg4), buffer, file, line); 

}

void debug_assert(unsigned int level, unsigned int test, char *file, unsigned int line)
{

    if (!test)
        debug(level, 11, "ASSERT FAIL", file, line);

}

void debug_registerinterface(struct debug_interface *interface)
{

    resource_register(&interface->resource);

}

void debug_unregisterinterface(struct debug_interface *interface)
{

    resource_unregister(&interface->resource);

}

void debug_initinterface(struct debug_interface *interface, void (*write)(unsigned int level, unsigned int count, char *string, char *file, unsigned int line))
{

    resource_init(&interface->resource, RESOURCE_DEBUGLOG, interface);

    interface->write = write;

}

