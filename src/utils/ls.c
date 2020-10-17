#include <fudge.h>
#include <abi.h>

static void print(struct channel *channel, unsigned int source)
{

    struct record record;

    file_open(FILE_G0);

    while (file_readall(FILE_G0, &record, sizeof (struct record)))
    {

        union message message;

        message_init(&message, EVENT_DATA);
        message_append(&message, record.length, record.name);
        message_appendstring(&message, "\n");
        channel_place(channel, &message, source);

        if (!file_step(FILE_G0))
            break;

    }

    file_close(FILE_G0);

}

static void onmain(struct channel *channel, unsigned int source, void *mdata, unsigned int msize)
{

    if (file_duplicate(FILE_G0, FILE_PW))
        print(channel, source);

    channel_close(channel);

}

static void onmain2(struct channel *channel, unsigned int source, void *mdata, unsigned int msize)
{

    channel_close(channel);

}

static void onfile(struct channel *channel, unsigned int source, void *mdata, unsigned int msize)
{

    if (file_walk2(FILE_G0, mdata))
        print(channel, source);

    channel_setsignal(channel, EVENT_MAIN, onmain2);

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
    channel_setsignal(&channel, EVENT_FILE, onfile);
    channel_setsignal(&channel, EVENT_REDIRECT, onredirect);
    channel_listen(&channel, 0, 0);

}

