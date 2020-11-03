#include <fudge.h>
#include <abi.h>

static void print(struct channel *channel, unsigned int source, unsigned int count, void *buffer)
{

    unsigned char *data = buffer;
    unsigned int i;

    for (i = 0; i < count; i += 16)
    {

        struct message_data message;
        unsigned int offset = 0;
        unsigned int j;

        offset = message_appendvalue(&message, i, 16, 8, offset);
        offset = message_appendstring(&message, "  ", offset);

        for (j = i; j < i + 16; j++)
        {

            if (j < count)
            {

                offset = message_appendvalue(&message, data[j], 16, 2, offset);
                offset = message_appendstring(&message, " ", offset);

            }

            else
            {

                offset = message_appendstring(&message, "   ", offset);

            }

        }

        offset = message_appendstring(&message, " |", offset);

        for (j = i; j < i + 16; j++)
        {

            if (j < count)
            {

                char c = data[j];

                if (!(c >= 0x20 && c <= 0x7e))
                    c = ' ';

                offset = message_append(&message, offset, 1, &c);

            }

            else
            {

                offset = message_appendstring(&message, " ", offset);

            }

        }

        offset = message_appendstring(&message, "|\n", offset);

        channel_place(channel, source, EVENT_DATA, offset, &message);

    }

}

static void ondata(struct channel *channel, unsigned int source, void *mdata, unsigned int msize)
{

    print(channel, source, msize, mdata);

}

static void onfile(struct channel *channel, unsigned int source, void *mdata, unsigned int msize)
{

    if (file_walk2(FILE_L0, mdata))
    {

        char buffer[FUDGE_MSIZE];
        unsigned int count;

        file_open(FILE_L0);

        while ((count = file_read(FILE_L0, buffer, FUDGE_MSIZE)))
            print(channel, source, count, buffer);

        file_close(FILE_L0);

    }

}

void main(void)
{

    struct channel channel;

    channel_init(&channel);
    channel_setsignal(&channel, EVENT_DATA, ondata);
    channel_setsignal(&channel, EVENT_FILE, onfile);
    channel_listen(&channel, 0, 0);

}

