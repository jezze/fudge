#include <abi.h>
#include <fudge.h>
#include "file.h"

unsigned int file_walk(unsigned int descriptor, char *path)
{

    if (path[0] == '/')
        return file_walkfrom(descriptor, CALL_PR, path + 1);
    else
        return file_walkfrom(descriptor, CALL_PW, path);

}

unsigned int file_walkfrom(unsigned int descriptor, unsigned int pdescriptor, char *path)
{

    return call_walk(descriptor, pdescriptor, path, ascii_length(path));

}

unsigned int file_duplicate(unsigned int descriptor, unsigned int pdescriptor)
{

    return call_walk(descriptor, pdescriptor, 0, 0);

}

unsigned int file_open(unsigned int descriptor)
{

    return call_open(descriptor);

}

unsigned int file_close(unsigned int descriptor)
{

    return call_close(descriptor);

}

unsigned int file_read(unsigned int descriptor, void *buffer, unsigned int count)
{

    return call_read(descriptor, buffer, count);

}

unsigned int file_readall(unsigned int descriptor, void *buffer, unsigned int count)
{

    unsigned char *b = buffer;
    unsigned int n;

    if (!count)
        return 0;

    while ((n = call_read(descriptor, b, count)))
    {

        count -= n;
        b += n;

    }

    return !count;

}

unsigned int file_write(unsigned int descriptor, void *buffer, unsigned int count)
{

    return call_write(descriptor, buffer, count);

}

unsigned int file_writeall(unsigned int descriptor, void *buffer, unsigned int count)
{

    unsigned char *b = buffer;
    unsigned int n;

    if (!count)
        return 0;

    while ((n = call_write(descriptor, b, count)))
    {

        count -= n;
        b += n;

    }

    return !count;

}

unsigned int file_seekread(unsigned int descriptor, void *buffer, unsigned int count, unsigned int offset)
{

    call_seek(descriptor, offset);

    return call_read(descriptor, buffer, count);

}

unsigned int file_seekreadall(unsigned int descriptor, void *buffer, unsigned int count, unsigned int offset)
{

    call_seek(descriptor, offset);

    return file_readall(descriptor, buffer, count);

}

unsigned int file_seekwrite(unsigned int descriptor, void *buffer, unsigned int count, unsigned int offset)
{

    call_seek(descriptor, offset);

    return call_write(descriptor, buffer, count);

}

unsigned int file_seekwriteall(unsigned int descriptor, void *buffer, unsigned int count, unsigned int offset)
{

    call_seek(descriptor, offset);

    return file_writeall(descriptor, buffer, count);

}

