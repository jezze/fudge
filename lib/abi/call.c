#include <fudge.h>
#include "call.h"

unsigned int call_walk_duplicate(unsigned int idescriptor, unsigned int pdescriptor)
{

    return call_walk(idescriptor, pdescriptor, 0, 0);

}

unsigned int call_walk_relative(unsigned int idescriptor, unsigned int pdescriptor, char *path)
{

    return call_walk(idescriptor, pdescriptor, path, cstring_length(path));

}

unsigned int call_walk_absolute(unsigned int idescriptor, char *path)
{

    return call_walk(idescriptor, FILE_PW, path, cstring_length(path));

}

unsigned int call_read_all(unsigned int idescriptor, void *buffer, unsigned int count, unsigned int offset)
{

    unsigned char *b = buffer;
    unsigned int c;

    for (c = 0; c < count; c += call_read(idescriptor, b + c, count - c, offset + c));

    return c;

}

unsigned int call_write_all(unsigned int idescriptor, void *buffer, unsigned int count, unsigned int offset)
{

    unsigned char *b = buffer;
    unsigned int c;

    for (c = 0; c < count; c += call_write(idescriptor, b + c, count - c, offset + c));

    return c;

}

unsigned int call_spawn_absolute(unsigned int pdescriptor, unsigned int wdescriptor, char *path)
{

    return (call_walk_absolute(pdescriptor, path)) ? call_spawn(pdescriptor, wdescriptor) : 0;

}

