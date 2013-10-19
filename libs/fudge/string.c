#include "string.h"

unsigned int string_length(const char *in)
{

    unsigned int offset;

    for (offset = 0; in[offset]; offset++);

    return offset;

}

