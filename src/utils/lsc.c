#include <fudge.h>
#include <abi.h>

static void print(struct channel *channel, unsigned int source)
{

    struct record record;

    file_open(FILE_G0);

    while (file_readall(FILE_G0, &record, sizeof (struct record)))
    {

        channel_request(channel, EVENT_DATA);
        channel_appendvalue(channel, record.id, 16, 8);
        channel_appendstring(channel, " ");
        channel_appendvalue(channel, record.size, 16, 8);
        channel_appendstring(channel, " ");
        channel_append(channel, record.length, record.name);
        channel_appendstring(channel, "\n");
        channel_place(channel, source);

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
    channel_listen(&channel);

}

