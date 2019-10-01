#include <fudge.h>
#include <abi.h>

static void dump(struct channel *channel, unsigned int count, void *buffer)
{

    unsigned int id = channel_reply(channel, EVENT_DATA);
    unsigned char *data = buffer;
    unsigned int i;

    for (i = 0; i < count; i += 16)
    {

        unsigned int j;

        channel_appendvalue(channel, i, 16, 8);
        channel_appendstring(channel, "  ");

        for (j = i; j < i + 16; j++)
        {

            if (j < count)
            {

                channel_appendvalue(channel, data[j], 16, 2);
                channel_appendstring(channel, " ");

            }

            else
            {

                channel_appendstring(channel, "   ");

            }

        }

        channel_appendstring(channel, " |");

        for (j = i; j < i + 16; j++)
        {

            if (j < count)
            {

                char c = data[j];

                if (!(c >= 0x20 && c <= 0x7e))
                    c = ' ';

                channel_append(channel, 1, &c);

            }

            else
            {

                channel_appendstring(channel, " ");

            }

        }

        channel_appendstring(channel, "|\n");

    }

    channel_place(channel, id);

}

static void ondata(struct channel *channel, void *mdata, unsigned int msize)
{

    dump(channel, msize, mdata);

}

static void onfile(struct channel *channel, void *mdata, unsigned int msize)
{

    struct event_file *file = mdata;
    char buffer[FUDGE_BSIZE];
    unsigned int count;

    file_open(file->descriptor);

    while ((count = file_read(file->descriptor, buffer, FUDGE_BSIZE)))
        dump(channel, count, buffer);

    file_close(file->descriptor);

}

void main(void)
{

    struct channel channel;

    channel_init(&channel);
    channel_setsignal(&channel, EVENT_DATA, ondata);
    channel_setsignal(&channel, EVENT_FILE, onfile);
    channel_listen(&channel);

}

