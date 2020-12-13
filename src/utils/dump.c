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

        channel_place(channel, EVENT_DATA, offset, &data);

        page += 16;

    }

}

static void ondata(struct channel *channel, unsigned int source, void *mdata, unsigned int msize)
{

    print(channel, source, msize, mdata);

}

static void onfile(struct channel *channel, unsigned int source, void *mdata, unsigned int msize)
{

    page = 0;

    if (file_walk2(FILE_L0, mdata))
    {

        struct message_data data;
        unsigned int count;

        file_open(FILE_L0);

        while ((count = file_read(FILE_L0, &data, sizeof (struct message_data))))
            print(channel, source, count, &data);

        file_close(FILE_L0);

    }

}

void init(struct channel *channel)
{

    channel_setcallback(channel, EVENT_DATA, ondata);
    channel_setcallback(channel, EVENT_FILE, onfile);

}

