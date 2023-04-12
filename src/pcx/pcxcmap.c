#include <fudge.h>
#include <abi.h>
#include <image.h>

static void onpath(unsigned int source, void *mdata, unsigned int msize)
{

    if (file_walk2(FILE_L0, mdata))
    {

        struct pcx_header header;
        struct record record;
        unsigned char magic;

        file_stat(FILE_L0, &record);
        file_readall(FILE_L0, &header, sizeof (struct pcx_header));
        file_seekreadall(FILE_L0, &magic, 1, record.size - 768 - 1);

        if (magic == PCX_COLORMAP_MAGIC)
        {

            unsigned char colormap[768];

            file_seekreadall(FILE_L0, colormap, 768, record.size - 768);
            channel_sendbuffer(CHANNEL_DEFAULT, EVENT_DATA, 768, colormap);

        }

    }

    else
    {

        channel_sendfmt1(CHANNEL_DEFAULT, EVENT_ERROR, "Path not found: %s\n", mdata);

    }

}

void init(void)
{

    channel_bind(EVENT_PATH, onpath);

}

