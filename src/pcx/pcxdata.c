#include <fudge.h>
#include <abi.h>
#include "pcx.h"

static unsigned int readline(unsigned int width, unsigned int offset, unsigned char *buffer)
{

    unsigned char raw[BUFFER_SIZE];
    unsigned int rindex = 0;
    unsigned int oindex = 0;

    file_seekread(FILE_L0, raw, BUFFER_SIZE, 128 + offset);

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

static void onpath(unsigned int source, void *mdata, unsigned int msize)
{

    if (file_walk2(FILE_L0, mdata))
    {

        struct pcx_header header;
        unsigned char magic;
        unsigned int width;
        unsigned int height;
        unsigned int filesize = 31467;
        unsigned int offset = 0;
        unsigned int h;

        file_seekreadall(FILE_L0, &header, sizeof (struct pcx_header), 0);

        width = header.xend - header.xstart + 1;
        height = header.yend - header.ystart + 1;

        file_seekreadall(FILE_L0, &magic, 1, filesize - 768 - 1);

        for (h = 0; h < height; h++)
        {

            unsigned char buffer[BUFFER_SIZE];

            offset = readline(width, offset, buffer);

            channel_reply(EVENT_DATA, width, buffer);

        }

    }

}

void init(void)
{

    channel_bind(EVENT_PATH, onpath);

}

