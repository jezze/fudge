#include <lib/string.h>
#include <kernel/log.h>

static char log[4096];

char *log_get()
{

    return log;

}

void log_string(char *str)
{

    string_concat(log, str);

}

unsigned int log_num(unsigned int num, unsigned int base)
{

    if (!num)
        log_string("0");

    char buffer[32] = {0};

    int i;

    for (i = 30; num && i; --i, num /= base)
        buffer[i] = "0123456789abcdef"[num % base];

    log_string(buffer + i + 1);

}

void log_init()
{

}

