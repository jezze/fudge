#include <lib/string.h>
#include <kernel/log.h>

static char log[4096];
unsigned int offset;

char *log_get()
{

    return log;

}

void log_write(char *buffer, ...)
{

    string_write_format(log + offset, buffer, &buffer + 4);
    offset += string_length(log + offset);

}

void log_init()
{

    offset = 0;

}

