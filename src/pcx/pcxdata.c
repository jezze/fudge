#include <fudge.h>
#include <abi.h>
#include <image.h>

static void onpath(unsigned int source, void *mdata, unsigned int msize)
{

    if (call_walk_absolute(FILE_L0, mdata))
    {

        struct pcx_header header;
        struct record record;
        unsigned int width;
        unsigned int height;
        unsigned int offset = 0;
        unsigned int h;

        call_stat(FILE_L0, &record);
        call_read_all(FILE_L0, &header, sizeof (struct pcx_header), 0);

        width = header.xend - header.xstart + 1;
        height = header.yend - header.ystart + 1;

        for (h = 0; h < height; h++)
        {

            unsigned char buffer[4096];
            unsigned char raw[4096];

            call_read(FILE_L0, raw, 4096, 128 + offset);

            offset = pcx_readline(raw, width, buffer);

            channel_send_buffer(CHANNEL_DEFAULT, EVENT_DATA, width, buffer);

        }

    }

    else
    {

        channel_send_fmt1(CHANNEL_DEFAULT, EVENT_ERROR, "Path not found: %s\n", mdata);

    }

}

void init(void)
{

    channel_bind(EVENT_PATH, onpath);

}

