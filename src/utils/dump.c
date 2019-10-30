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

static void ondone(struct channel *channel, void *mdata, unsigned int msize)
{

    channel_close(channel);

}

static void ondata(struct channel *channel, void *mdata, unsigned int msize)
{

    dump(channel, msize, mdata);

}

static void onfile(struct channel *channel, void *mdata, unsigned int msize)
{

    char buffer[FUDGE_BSIZE];
    unsigned int count;

    if (!file_walk2(FILE_L0, mdata))
        return;

    file_open(FILE_L0);

    while ((count = file_read(FILE_L0, buffer, FUDGE_BSIZE)))
        dump(channel, count, buffer);

    file_close(FILE_L0);

}

void main(void)
{

    struct channel channel;

    channel_init(&channel);
    channel_setsignal(&channel, EVENT_DONE, ondone);
    channel_setsignal(&channel, EVENT_DATA, ondata);
    channel_setsignal(&channel, EVENT_FILE, onfile);
    channel_listen(&channel);

}

