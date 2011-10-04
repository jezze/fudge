#include <lib/string.h>
#include <kernel/log.h>

static char log[4096];
unsigned int logCount;

char *log_get()
{

    return log;

}

static void log_putc(char c)
{

    log[logCount] = c;
    log[logCount + 1] = '\0';
    logCount++;

}

static void log_puts(char *str)
{

    while (*str)
        log_putc(*str++);

}

static unsigned int log_num(unsigned int num, unsigned int base)
{

    if (!num)
        log_puts("0");

    char buffer[32] = {0};

    int i;

    for (i = 30; num && i; --i, num /= base)
        buffer[i] = "0123456789abcdef"[num % base];

    log_puts(buffer + i + 1);

}

void log_write(const char *buffer, ...)
{

    char **arg = (char **)&buffer;
    arg++;

    char c;

    while ((c = *buffer++))
    {

        if (c != '%')
        {

            log_putc(c);
            continue;

        }

        c = *buffer++;

        switch (c)
        {

            case 'b':

                log_num(*(char *)arg, 10);

                break;

            case 'd':

                log_num(*(int *)arg, 10);

                break;

            case 's':

                log_puts(*(char **)arg);

                break;

            case 'w':

                log_num(*(short *)arg, 16);

                break;

            case 'x':

                log_num(*(int *)arg, 16);

                break;

        }

        arg++;

    }

}

void log_init()
{

    logCount = 0;

}

