#include <fudge.h>
#include <abi.h>

static unsigned int page;

static void print(struct channel *channel, unsigned int source, unsigned int count, void *buffer)
{

    unsigned char *b = buffer;
    unsigned int i;

    for (i = 0; i < count; i += 16)
    {

        struct message_data data;
        unsigned int offset = 0;
        unsigned int j;

        offset = message_putvalue(&data, page, 16, 8, offset);
        offset = message_putstring(&data, "  ", offset);

        for (j = i; j < i + 16; j++)
        {

            if (j < count)
            {

                offset = message_putvalue(&data, b[j], 16, 2, offset);
                offset = message_putstring(&data, " ", offset);

            }

            else
            {

                offset = message_putstring(&data, "   ", offset);

            }

        }

        offset = message_putstring(&data, " |", offset);

        for (j = i; j < i + 16; j++)
        {

            if (j < count)
            {

                char c = b[j];

                if (!(c >= 0x20 && c <= 0x7e))
                    c = ' ';

                offset = message_putbuffer(&data, 1, &c, offset);

            }

            else
            {

                offset = message_putstring(&data, " ", offset);

            }

        }

        offset = message_putstring(&data, "|\n", offset);

        channel_place(channel, source, EVENT_DATA, offset, &data);

        page += 16;

    }

}

static void onmain(struct channel *channel, unsigned int source, void *mdata, unsigned int msize)
{

    struct message_header header;
    struct message_data data;

    file_link(FILE_G0);

    while (channel_pollsource(channel, 0, &header, &data))
    {

        if (header.event == EVENT_DATA)
            print(channel, source, message_datasize(&header), &data);

    }

    file_unlink(FILE_G0);
    channel_close(channel, source);

}

void init(struct channel *channel)
{

    if (!file_walk2(FILE_G0, "/system/ethernet/if:0/data"))
        return;

    channel_setcallback(channel, EVENT_MAIN, onmain);

}

