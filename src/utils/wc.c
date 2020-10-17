#include <fudge.h>
#include <abi.h>

static unsigned int bytes;
static unsigned int words;
static unsigned int lines;
static unsigned int whitespace = 1;

static void sum(unsigned int count, void *buffer)
{

    char *data = buffer;
    unsigned int i;

    for (i = 0; i < count; i++)
    {

        switch (data[i])
        {

        case '\n':
            whitespace = 1;
            lines++;

            break;

        case ' ':
            whitespace = 1;

            break;

        default:
            if (whitespace)
                words++;

            whitespace = 0;

            break;

        }

        bytes++;

    }

}

static void onmain(struct channel *channel, unsigned int source, void *mdata, unsigned int msize)
{

    struct message_data message;
    unsigned int offset = 0;

    offset = message_appendvalue(&message, lines, 10, 0, offset);
    offset = message_appendstring(&message, "\n", offset);
    offset = message_appendvalue(&message, words, 10, 0, offset);
    offset = message_appendstring(&message, "\n", offset);
    offset = message_appendvalue(&message, bytes, 10, 0, offset);
    offset = message_appendstring(&message, "\n", offset);

    channel_place(channel, source, EVENT_DATA, offset, &message);
    channel_close(channel);

}

static void ondata(struct channel *channel, unsigned int source, void *mdata, unsigned int msize)
{

    sum(msize, mdata);

}

static void onfile(struct channel *channel, unsigned int source, void *mdata, unsigned int msize)
{

    if (file_walk2(FILE_L0, mdata))
    {

        char buffer[FUDGE_BSIZE];
        unsigned int count;

        file_open(FILE_L0);

        while ((count = file_read(FILE_L0, buffer, FUDGE_BSIZE)))
            sum(count, buffer);

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
    channel_listen(&channel, 0, 0);

}

