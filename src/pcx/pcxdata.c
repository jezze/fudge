#include <fudge.h>
#include <abi.h>
#include <image.h>

static void onpath(unsigned int source, void *mdata, unsigned int msize)
{

    if (file_walk2(FILE_L0, mdata))
    {

        struct pcx_header header;
        struct record record;
        unsigned int width;
        unsigned int height;
        unsigned int offset = 0;
        unsigned int h;

        file_stat(FILE_L0, &record);
        file_readall(FILE_L0, &header, sizeof (struct pcx_header));

        width = header.xend - header.xstart + 1;
        height = header.yend - header.ystart + 1;

        for (h = 0; h < height; h++)
        {

            unsigned char buffer[BUFFER_SIZE];
            unsigned char raw[BUFFER_SIZE];

            file_seekread(FILE_L0, raw, BUFFER_SIZE, 128 + offset);

            offset = pcx_readline(raw, width, buffer);

            channel_sendbuffer(CHANNEL_DEFAULT, EVENT_DATA, width, buffer);

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

