#include <lib/memory.h>
#include <lib/string.h>
#include <kernel/log.h>
#include <kernel/vfs.h>

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

void log_write(const char *buffer, ...)
{

    struct vfs_filesystem *filesystem = vfs_find_filesystem("module/tty/stdout");

    if (!filesystem)
        return;

    struct vfs_node *node = filesystem->get_node(filesystem, filesystem->find_node(filesystem, "module/tty/stdout"));

    char **arg = (char **)&buffer;

    char num[32];
    char c = '\0';

    while ((c = *buffer))
    {

        if (c != '%')
        {

            node->write(node, 1, &c);

            buffer++;

            continue;

        }

        arg++;
        buffer++;

        c = *buffer;

        switch (c)
        {

            case 'c':

                node->write(node, 1, (char *)arg);

                break;

            case 'd':

                log_write_num(num, *(int *)arg, 10);
                node->write(node, string_length(num), num);

                break;

            case 's':

                node->write(node, string_length(*(char **)arg), *(char **)arg);

                break;

            case 'x':

                log_write_num(num, *(int *)arg, 16);
                node->write(node, string_length(num), num);

                break;

        }

        buffer++;

    }

    node->write(node, 1, "");

}

