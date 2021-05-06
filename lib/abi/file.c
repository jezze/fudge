#include <fudge.h>
#include "call.h"
#include "file.h"

unsigned int file_walk(unsigned int descriptor, unsigned int pdescriptor, char *path)
{

    return call_walk(descriptor, pdescriptor, path, ascii_length(path));

}

unsigned int file_walk2(unsigned int descriptor, char *path)
{

    if (path[0] == '/')
        return call_walk(descriptor, FILE_PR, path + 1, ascii_length(path + 1));
    else
        return call_walk(descriptor, FILE_PW, path, ascii_length(path));

}

unsigned int file_duplicate(unsigned int descriptor, unsigned int pdescriptor)
{

    return call_walk(descriptor, pdescriptor, 0, 0);

}

unsigned int file_create(unsigned int descriptor, unsigned int pdescriptor, char *name)
{

    return call_create(descriptor, pdescriptor, name, ascii_length(name));

}

unsigned int file_reset(unsigned int descriptor)
{

    return call_reset(descriptor);

}

unsigned int file_seek(unsigned int descriptor, unsigned int offset)
{

    return call_seek(descriptor, offset);

}

unsigned int file_step(unsigned int descriptor)
{

    return call_step(descriptor);

}

unsigned int file_read(unsigned int descriptor, void *buffer, unsigned int count)
{

    return call_read(descriptor, buffer, count);

}

unsigned int file_readall(unsigned int descriptor, void *buffer, unsigned int count)
{

    unsigned char *b = buffer;
    unsigned int c;

    for (c = 0; c < count; c += call_read(descriptor, b + c, count - c));

    return c;

}

unsigned int file_write(unsigned int descriptor, void *buffer, unsigned int count)
{

    return call_write(descriptor, buffer, count);

}

unsigned int file_writeall(unsigned int descriptor, void *buffer, unsigned int count)
{

    unsigned char *b = buffer;
    unsigned int c;

    for (c = 0; c < count; c += call_write(descriptor, b + c, count - c));

    return c;

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

unsigned int file_spawn(char *path)
{

    if (file_walk2(FILE_CP, path))
        return call_spawn();

    return 0;

}

unsigned int file_link(unsigned int descriptor)
{

    return call_link(descriptor);

}

unsigned int file_unlink(unsigned int descriptor)
{

    return call_unlink(descriptor);

}

unsigned int file_notify(unsigned int descriptor, unsigned int event, unsigned int count, void *data)
{

    return call_notify(descriptor, event, count, data);

}

