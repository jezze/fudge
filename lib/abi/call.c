#include <fudge.h>
#include "call.h"

unsigned int call_walk_duplicate(unsigned int descriptor, unsigned int pdescriptor)
{

    return call_walk(descriptor, pdescriptor, 0, 0);

}

unsigned int call_walk_relative(unsigned int descriptor, unsigned int pdescriptor, char *path)
{

    return call_walk(descriptor, pdescriptor, path, cstring_length(path));

}

unsigned int call_walk_absolute(unsigned int descriptor, char *path)
{

    return call_walk(descriptor, FILE_PW, path, cstring_length(path));

}

unsigned int call_read_all(unsigned int descriptor, void *buffer, unsigned int count, unsigned int offset)
{

    unsigned char *b = buffer;
    unsigned int c;

    for (c = 0; c < count; c += call_read(descriptor, b + c, count - c, offset + c));

    return c;

}

unsigned int call_write_all(unsigned int descriptor, void *buffer, unsigned int count, unsigned int offset)
{

    unsigned char *b = buffer;
    unsigned int c;

    for (c = 0; c < count; c += call_write(descriptor, b + c, count - c, offset + c));

    return c;

}

unsigned int call_spawn_absolute(unsigned int pdescriptor, unsigned int wdescriptor, char *path)
{

    return (call_walk_absolute(pdescriptor, path)) ? call_spawn(pdescriptor, wdescriptor) : 0;

}

