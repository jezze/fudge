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

void log_write(const char *buffer, ...)
{

    char **arg = (char **)&buffer;
    arg++;

    char num[32];
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

            case 'c':

                log_putc(*(char *)arg);

                break;

            case 'd':

                string_copy_num(num, *(int *)arg, 10);
                log_puts(num);

                break;

            case 's':

                log_puts(*(char **)arg);

                break;

            case 'x':

                string_copy_num(num, *(int *)arg, 16);
                log_puts(num);

                break;

        }

        arg++;

    }

}

void log_init()
{

    logCount = 0;

}

