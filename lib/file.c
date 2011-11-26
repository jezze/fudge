#include <lib/call.h>
#include <lib/file.h>
#include <lib/memory.h>
#include <lib/string.h>

void file_close(unsigned int fd)
{

    return call_close(fd);

}

unsigned int file_open(char *view, char *name)
{

    return call_open(view, name);

}

unsigned int file_read(unsigned int fd, unsigned int count, void *buffer)
{

    return call_read(fd, buffer, count);

}

unsigned int file_read_byte(unsigned int fd, char c)
{

    return file_read(fd, 1, &c);

}

unsigned int file_write(unsigned int fd, unsigned int count, void *buffer)
{

    return call_write(fd, buffer, count);

}

unsigned int file_write_byte(unsigned int fd, char c)
{

    return file_write(fd, 1, &c);

}

static unsigned int file_write_string(unsigned int fd, char *buffer)
{

    return file_write(fd, string_length(buffer), buffer);

}

static unsigned int file_write_num(unsigned int fd, unsigned int num, unsigned int base)
{

    if (!num)
        return file_write_string(fd, "0");

    char buffer[32];
    memory_clear(buffer, 32);

    int i;

    for (i = 30; num && i; --i, num /= base)
        buffer[i] = "0123456789abcdef"[num % base];

    return file_write_string(fd, buffer + i + 1);

}

unsigned int file_write_format(unsigned int fd, char *buffer, ...)
{

    char **arg = (char **)&buffer;
    arg++;

    int count = 0;
    char c;

    while ((c = *buffer++))
    {

        if (c != '%')
        {

            count += file_write_byte(fd, c);
            continue;

        }

        c = *buffer++;

        switch (c)
        {

            case 'c':

                count += file_write_byte(fd, *(char *)arg);

                break;

            case 'd':

                count += file_write_num(fd, *(int *)arg, 10);

                break;

            case 's':

                count += file_write_string(fd, *(char **)arg);

                break;

            case 'x':

                count += file_write_num(fd, *(int *)arg, 16);

                break;

        }

        arg++;

    }

    return count;

}

