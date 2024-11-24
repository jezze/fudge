#include <fudge.h>
#include <abi.h>
#include <image.h>

static void onpath(unsigned int source, void *mdata, unsigned int msize)
{

    unsigned int service = fs_auth(mdata);
    unsigned int id = fs_walk(1, service, 0, mdata);

    if (id)
    {

        struct pcx_header header;
        struct record record;
        unsigned int width;
        unsigned int height;
        unsigned int offset = 0;
        unsigned int h;

        fs_stat(1, service, id, &record);
        fs_read_all(1, service, id, &header, sizeof (struct pcx_header), 0);

        width = header.xend - header.xstart + 1;
        height = header.yend - header.ystart + 1;

        for (h = 0; h < height; h++)
        {

            unsigned char buffer[4096];
            unsigned char raw[4096];

            fs_read_full(1, service, id, raw, 4096, 128 + offset);

            offset = pcx_readline(raw, width, buffer);

            channel_send_buffer(0, source, EVENT_DATA, width, buffer);

        }

    }

    else
    {

        channel_send_fmt1(0, source, EVENT_ERROR, "Path not found: %s\n", mdata);

    }

}

void init(void)
{

    channel_bind(EVENT_PATH, onpath);

}

