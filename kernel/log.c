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

    struct vfs_filesystem *filesystem = vfs_find_filesystem("module/");

    if (!filesystem)
        return;

    unsigned int id = filesystem->find(filesystem, "module/tty/stdout");

    if (!id)
        return;

    char **arg = (char **)&buffer;

    char num[32];
    char c = '\0';

    while ((c = *buffer))
    {

        if (c != '%')
        {

            filesystem->write(filesystem, id, 1, &c);

            buffer++;

            continue;

        }

        arg++;
        buffer++;

        c = *buffer;

        switch (c)
        {

            case 'c':

                filesystem->write(filesystem, id, 1, (char *)arg);

                break;

            case 'd':

                log_write_num(num, *(int *)arg, 10);
                filesystem->write(filesystem, id, string_length(num), num);

                break;

            case 's':

                filesystem->write(filesystem, id, string_length(*(char **)arg), *(char **)arg);

                break;

            case 'x':

                log_write_num(num, *(int *)arg, 16);
                filesystem->write(filesystem, id, string_length(num), num);

                break;

        }

        buffer++;

    }

    filesystem->write(filesystem, id, 1, "");

}

