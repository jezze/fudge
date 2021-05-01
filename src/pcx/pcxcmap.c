#include <fudge.h>
#include <abi.h>
#include "pcx.h"

static void onpath(struct channel *channel, unsigned int source, void *mdata, unsigned int msize)
{

    if (file_walk2(FILE_L0, mdata))
    {

        struct pcx_header header;
        unsigned char magic;
        unsigned int filesize = 31467;

        file_open(FILE_L0);
        file_seekreadall(FILE_L0, &header, sizeof (struct pcx_header), 0);
        file_seekreadall(FILE_L0, &magic, 1, filesize - 768 - 1);

        if (magic == PCX_COLORMAP_MAGIC)
        {

            unsigned char colormap[768];

            file_seekreadall(FILE_L0, colormap, 768, filesize - 768);
            channel_reply(channel, EVENT_DATA, 768, colormap);

        }

        file_close(FILE_L0);

    }

}

void init(struct channel *channel)
{

    channel_setcallback(channel, EVENT_PATH, onpath);

}

