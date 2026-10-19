#include "pcx.h"

static short convert16(short value)
{

    unsigned char *bytes = (unsigned char *)&value;

    return (bytes[1] << 8) | bytes[0];

}

/* the header's words are little-endian on disk; this turns them into the cpu's order */
void pcx_convertheader(struct pcx_header *header)
{

    header->xstart = convert16(header->xstart);
    header->ystart = convert16(header->ystart);
    header->xend = convert16(header->xend);
    header->yend = convert16(header->yend);
    header->xdpi = convert16(header->xdpi);
    header->ydpi = convert16(header->ydpi);
    header->bpl = convert16(header->bpl);
    header->palettemode = convert16(header->palettemode);
    header->xscreen = convert16(header->xscreen);
    header->yscreen = convert16(header->yscreen);

}

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

