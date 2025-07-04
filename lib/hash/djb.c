#include <fudge.h>
#include "djb.h"

unsigned int djb_hash(char *buffer)
{

    unsigned int hash = 5381;
    int c;

    while ((c = *buffer++))
        hash = ((hash << 5) + hash) + c;

    return hash;

}

