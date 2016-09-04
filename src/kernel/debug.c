#include <fudge.h>
#include "resource.h"
#include "debug.h"

void debug_write(unsigned int level, char *string, char *file, unsigned int line)
{

    struct resource *current = 0;

    while ((current = resource_findtype(current, RESOURCE_DEBUGLOG)))
    {

        struct debug_log *log = current->data;

        log->write(level, string, file, line);

    }

}

void debug_initlog(struct debug_log *log, void (*write)(unsigned int level, char *string, char *file, unsigned int line))
{

    resource_init(&log->resource, RESOURCE_DEBUGLOG, log);

    log->write = write;

}

