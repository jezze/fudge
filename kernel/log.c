#include <lib/string.h>
#include <kernel/log.h>

static char log[4096];

char *log_get()
{

    return log;

}

void log_write(const char *buffer, ...)
{

}

void log_init()
{

}

