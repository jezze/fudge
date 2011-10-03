#include <lib/string.h>
#include <kernel/log.h>

static char log[4096];

char *log_get()
{

    return log;

}

void log_message(unsigned int type, char *message, void **args)
{

    string_concat(log, message);

}

void log_init()
{

}

