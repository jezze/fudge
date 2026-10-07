#include <fudge.h>
#include <abi.h>
#include "pcx.h"

unsigned int pcx_readline(unsigned char *raw, unsigned int count, unsigned char *buffer)
{

    unsigned int rindex = 0;
    unsigned int oindex = 0;

    while (oindex < count)
    {

        unsigned int repeat = 1;
        unsigned char current = raw[rindex++];

        if ((current & 0xC0) == 0xC0)
        {

            repeat = current & 0x3F;
            current = raw[rindex++];

        }

        while (repeat-- && oindex < count)
            buffer[oindex++] = current;

    }

    return rindex;

}

