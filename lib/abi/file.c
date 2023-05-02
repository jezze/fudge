#include <fudge.h>
#include "call.h"
#include "file.h"

unsigned int file_walk(unsigned int descriptor, unsigned int pdescriptor, char *path)
{

    return call_walk(descriptor, pdescriptor, path, cstring_length(path));

}

unsigned int file_walk2(unsigned int descriptor, char *path)
{

    return call_walk(descriptor, FILE_PW, path, cstring_length(path));

}

unsigned int file_duplicate(unsigned int descriptor, unsigned int pdescriptor)
{

    return call_walk(descriptor, pdescriptor, 0, 0);

}

unsigned int file_create(unsigned int descriptor, unsigned int pdescriptor, char *name)
{

    return call_create(descriptor, pdescriptor, name, cstring_length(name));

}

unsigned int file_stat(unsigned int descriptor, struct record *record)
{

    return call_stat(descriptor, record);

}

unsigned int file_list(unsigned int descriptor, unsigned int cid, unsigned int count, struct record *records)
{

    return call_list(descriptor, cid, count, records);

}

unsigned int file_read(unsigned int descriptor, void *buffer, unsigned int count, unsigned int offset)
{

    return call_read(descriptor, buffer, count, offset);

}

unsigned int file_readall(unsigned int descriptor, void *buffer, unsigned int count, unsigned int offset)
{

    unsigned char *b = buffer;
    unsigned int c;

    for (c = 0; c < count; c += call_read(descriptor, b + c, count - c, offset + c));

    return c;

}

unsigned int file_write(unsigned int descriptor, void *buffer, unsigned int count, unsigned int offset)
{

    return call_write(descriptor, buffer, count, offset);

}

unsigned int file_writeall(unsigned int descriptor, void *buffer, unsigned int count, unsigned int offset)
{

    unsigned char *b = buffer;
    unsigned int c;

    for (c = 0; c < count; c += call_write(descriptor, b + c, count - c, offset + c));

    return c;

}

unsigned int file_spawn(unsigned int pdescriptor, char *path)
{

    return (file_walk2(pdescriptor, path)) ? call_spawn(pdescriptor, FILE_PW) : 0;

}

unsigned int file_spawn2(unsigned int pdescriptor, unsigned int wdescriptor, char *path)
{

    return (file_walk2(pdescriptor, path)) ? call_spawn(pdescriptor, wdescriptor) : 0;

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

