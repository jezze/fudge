#include <abi.h>
#include <fudge.h>
#include "file.h"

unsigned int file_open(unsigned int id)
{

    return call_open(id);

}

unsigned int file_close(unsigned int id)
{

    return call_close(id);

}

unsigned int file_read(unsigned int id, void *buffer, unsigned int count)
{

    return call_read(id, buffer, count);

}

unsigned int file_readall(unsigned int id, void *buffer, unsigned int count)
{

    unsigned char *b = buffer;
    unsigned int n;

    while ((n = call_read(id, b, count)))
    {

        count -= n;
        b += n;

    }

    return !count;

}

unsigned int file_write(unsigned int id, void *buffer, unsigned int count)
{

    return call_write(id, buffer, count);

}

unsigned int file_writeall(unsigned int id, void *buffer, unsigned int count)
{

    unsigned char *b = buffer;
    unsigned int n;

    while ((n = call_write(id, b, count)))
    {

        count -= n;
        b += n;

    }

    return !count;

}

unsigned int file_seekread(unsigned int id, void *buffer, unsigned int count, unsigned int offset)
{

    call_seek(id, offset);

    return call_read(id, buffer, count);

}

unsigned int file_seekreadall(unsigned int id, void *buffer, unsigned int count, unsigned int offset)
{

    call_seek(id, offset);

    return file_readall(id, buffer, count);

}

unsigned int file_seekwrite(unsigned int id, void *buffer, unsigned int count, unsigned int offset)
{

    call_seek(id, offset);

    return call_write(id, buffer, count);

}

unsigned int file_seekwriteall(unsigned int id, void *buffer, unsigned int count, unsigned int offset)
{

    call_seek(id, offset);

    return file_writeall(id, buffer, count);

}

