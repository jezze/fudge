#include <fudge.h>
#include <abi.h>

static void print(struct channel *channel, unsigned int source, unsigned int count, void *buffer)
{

    unsigned char *data = buffer;
    unsigned int i;

    for (i = 0; i < count; i += 16)
    {

        union message message;
        unsigned int j;

        message_init(&message, EVENT_DATA);
        message_appendvalue(&message, i, 16, 8);
        message_appendstring(&message, "  ");

        for (j = i; j < i + 16; j++)
        {

            if (j < count)
            {

                message_appendvalue(&message, data[j], 16, 2);
                message_appendstring(&message, " ");

            }

            else
            {

                message_appendstring(&message, "   ");

            }

        }

        message_appendstring(&message, " |");

        for (j = i; j < i + 16; j++)
        {

            if (j < count)
            {

                char c = data[j];

                if (!(c >= 0x20 && c <= 0x7e))
                    c = ' ';

                message_append(&message, 1, &c);

            }

            else
            {

                message_appendstring(&message, " ");

            }

        }

        message_appendstring(&message, "|\n");
        channel_place(channel, &message, source);

    }

}

static void onmain(struct channel *channel, unsigned int source, void *mdata, unsigned int msize)
{

    channel_close(channel);

}

static void ondata(struct channel *channel, unsigned int source, void *mdata, unsigned int msize)
{

    print(channel, source, msize, mdata);

}

static void onfile(struct channel *channel, unsigned int source, void *mdata, unsigned int msize)
{

    if (file_walk2(FILE_L0, mdata))
    {

        char buffer[FUDGE_BSIZE - 16];
        unsigned int count;

        file_open(FILE_L0);

        while ((count = file_read(FILE_L0, buffer, FUDGE_BSIZE - 16)))
            print(channel, source, count, buffer);

        file_close(FILE_L0);

    }

}

static void onredirect(struct channel *channel, unsigned int source, void *mdata, unsigned int msize)
{

    struct event_redirect *redirect = mdata;

    channel_setredirect(channel, redirect->type, redirect->mode, redirect->id, source);

}

void main(void)
{

    struct channel channel;

    channel_init(&channel);
    channel_setsignal(&channel, EVENT_MAIN, onmain);
    channel_setsignal(&channel, EVENT_DATA, ondata);
    channel_setsignal(&channel, EVENT_FILE, onfile);
    channel_setsignal(&channel, EVENT_REDIRECT, onredirect);
    channel_listen(&channel);

}

