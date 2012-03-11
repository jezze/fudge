#include <lib/call.h>
#include <lib/file.h>
#include <lib/memory.h>
#include <lib/string.h>

void file_close(unsigned int fd)
{

    return call_close(fd);

}

unsigned int file_open(char *path)
{

    return call_open(path);

}

unsigned int file_read(unsigned int fd, unsigned int count, void *buffer)
{

    return call_read(fd, 0, count, buffer);

}

unsigned int file_write(unsigned int fd, unsigned int count, void *buffer)
{

    return call_write(fd, 0, count, buffer);

}

unsigned int file_write_format(unsigned int fd, char *buffer, ...)
{

    char temp[0x1000];
    void **arg = (void **)&buffer + 1;

    string_write(temp, buffer, *arg);

    unsigned int count = string_length(temp) + 1;

    return call_write(fd, 0, count, temp);

}

