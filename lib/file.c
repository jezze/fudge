#include <lib/call.h>
#include <lib/file.h>
#include <lib/memory.h>
#include <lib/string.h>

// START REMOVE

struct file_node *file_find(struct file_node *node, char *path)
{

    unsigned int index = string_index(path, '/', 0);
    unsigned int length = string_length(path);

    if (!index)
        return node;

    struct file_node *current;
    unsigned int i;

    for (i = 0; (current = node->walk(node, i)); i++)
    {

        unsigned int count = string_length(current->name);

        if (index > count)
            count = index;

        if (!memory_compare(path, current->name, count))
            return (index == length) ? current : file_find(current, path + index + 1);

    }

    return 0;

}

// END REMOVE

void file_close(unsigned int fd)
{

    return call_close(fd);

}

int file_open(char *name)
{

    return call_open(name);

}

unsigned int file_read(unsigned int fd, unsigned int offset, unsigned int count, void *buffer)
{

    return call_read(fd, buffer, count);

}

unsigned int file_read_byte(unsigned int fd, char c)
{

    return file_read(fd, 0, 1, &c);

}

unsigned int file_write(unsigned int fd, unsigned int offset, unsigned int count, void *buffer)
{

    return call_write(fd, buffer, count);

}

unsigned int file_write_bcd(unsigned int fd, unsigned char num)
{

    return file_write_dec(fd, num >> 4) + file_write_dec(fd, num & 0x0F);

}

unsigned int file_write_byte(unsigned int fd, char c)
{

    return file_write(fd, 0, 1, &c);

}

unsigned int file_write_dec(unsigned int fd, unsigned int num)
{

    return file_write_num(fd, num, 10);

}

unsigned int file_write_hex(unsigned int fd, unsigned int num)
{

    return file_write_num(fd, num, 16);

}

unsigned int file_write_num(unsigned int fd, unsigned int num, unsigned int base)
{

    if (!num)
        return file_write_byte(fd, '0');

    char buffer[32] = {0};

    int i;

    for (i = 30; num && i; --i, num /= base)
        buffer[i] = "0123456789abcdef"[num % base];

    return file_write_string(fd, buffer + i + 1);

}

unsigned int file_write_string(unsigned int fd, char *buffer)
{

    return file_write(fd, 0, string_length(buffer), buffer);

}

unsigned int file_write_string_format(unsigned int fd, char *buffer, void **args)
{

    if (!args)
        return file_write(fd, 0, string_length(buffer), buffer);

    unsigned int i;
    unsigned int length = string_length(buffer);
    unsigned int size = 0;

    for (i = 0; i < length; i++)
    {

        if (buffer[i] != '%')
        {

            size += file_write_byte(fd, buffer[i]);

            continue;

        }

        i++;

        switch (buffer[i])
        {

            case 'c':

                size += file_write_byte(fd, **(char **)args);

                break;

            case 'd':

                size += file_write_num(fd, **(int **)args, 10);

                break;

            case 's':

                size += file_write_string(fd, *(char **)args);

                break;

            case 'x':

                size += file_write_num(fd, **(int **)args, 16);

                break;

        }

        args++;

    }

    return size + i;

}

