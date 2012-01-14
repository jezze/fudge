#include <lib/memory.h>
#include <lib/string.h>
#include <kernel/log.h>
#include <kernel/vfs.h>

static char log[512];

static char *log_write_num(char *out, unsigned int num, unsigned int base)
{

    if (!num)
        return string_write(out, "0");

    char buffer[32];
    memory_clear(buffer, 32);

    int i;

    for (i = 30; num && i; --i, num /= base)
        buffer[i] = "0123456789abcdef"[num % base];

    return string_write(out, buffer + i + 1);

}

void log_write(char *in, ...)
{

    char *out = log;
    char **arg = (char **)&in;
    arg++;

    char num[32];
    char c;

    while ((c = *in++))
    {

        if (c != '%')
        {

            memory_copy(out, &c, 1);
            out += 1;

            continue;

        }

        c = *in++;

        switch (c)
        {

            case 'c':

                memory_copy(out, (char *)arg, 1);
                out += 1;

                break;

            case 'd':

                log_write_num(num, *(int *)arg, 10);
                string_write(out, num);
                out += string_length(num);

                break;

            case 's':

                string_write(out, *(char **)arg);
                out += string_length(*(char **)arg);

                break;

            case 'x':

                log_write_num(num, *(int *)arg, 16);
                string_write(out, num);
                out += string_length(num);

                break;

        }

        arg++;

    }

    *(out) = '\0';

    struct vfs_node *node = vfs_find("tty", "stdout");

    if (node)
        node->write(node, string_length(log), log);

}

