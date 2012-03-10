#include <lib/memory.h>
#include <lib/string.h>
#include <kernel/log.h>
#include <kernel/vfs.h>

static char *log_write_num(char *out, unsigned int num, unsigned int base)
{

    if (!num)
        return string_write(out, "%s", "0");

    char buffer[32];
    memory_clear(buffer, 32);

    int i;

    for (i = 30; num && i; --i, num /= base)
        buffer[i] = "0123456789abcdef"[num % base];

    return string_write(out, "%s", buffer + i + 1);

}

void log_write(const char *buffer, ...)
{

    struct vfs_mount *mount = vfs_find_mount("/module/");

    if (!mount)
        return;

    unsigned int id = mount->filesystem->find(mount->filesystem, "tty/stdout");

    if (!id)
        return;

    char **arg = (char **)&buffer;

    char num[32];
    char c = '\0';

    while ((c = *buffer))
    {

        if (c != '%')
        {

            mount->filesystem->write(mount->filesystem, id, 0, 1, &c);

            buffer++;

            continue;

        }

        arg++;
        buffer++;

        c = *buffer;

        switch (c)
        {

            case 'c':

                mount->filesystem->write(mount->filesystem, id, 0, 1, (char *)arg);

                break;

            case 'd':

                log_write_num(num, *(int *)arg, 10);
                mount->filesystem->write(mount->filesystem, id, 0, string_length(num), num);

                break;

            case 's':

                mount->filesystem->write(mount->filesystem, id, 0, string_length(*(char **)arg), *(char **)arg);

                break;

            case 'x':

                log_write_num(num, *(int *)arg, 16);
                mount->filesystem->write(mount->filesystem, id, 0, string_length(num), num);

                break;

        }

        buffer++;

    }

    mount->filesystem->write(mount->filesystem, id, 0, 1, "");

}

