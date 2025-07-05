#include <fudge.h>
#include "djb.h"

unsigned int djb_hash(unsigned int count, char *buffer)
{

    unsigned int hash = 5381;
    unsigned int i;

    for (i = 0; i < count; i++)
        hash = ((hash << 5) + hash) + buffer[i];

    return hash;

}

