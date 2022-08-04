#include <fudge.h>
#include <abi.h>
#include "pcx.h"

unsigned int pcx_readline(unsigned char *raw, unsigned int width, unsigned char *buffer)
{

    unsigned int rindex = 0;
    unsigned int oindex = 0;

    do
    {

        unsigned int repeat = 1;
        unsigned char current = raw[rindex];

        rindex++;

        if ((current & 0xC0) == 0xC0)
        {

            repeat = current & 0x3F;
            current = raw[rindex];
            rindex++;

        }

        while (repeat--)
        {

            buffer[oindex] = current;
            oindex++;

        }

    } while (oindex < width);

    return rindex;

}

