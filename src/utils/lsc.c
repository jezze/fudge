#include <fudge.h>
#include <abi.h>

static void list(struct channel *channel, unsigned int source, unsigned int descriptor)
{

    struct record record;

    file_open(descriptor);

    while (file_readall(descriptor, &record, sizeof (struct record)))
    {

        channel_request(channel, EVENT_DATA);
        channel_appendvalue(channel, record.id, 16, 8);
        channel_appendstring(channel, " ");
        channel_appendvalue(channel, record.size, 16, 8);
        channel_appendstring(channel, " ");
        channel_append(channel, record.length, record.name);
        channel_appendstring(channel, "\n");
        channel_place(channel, source);

        if (!file_step(descriptor))
            break;

    }

    file_close(descriptor);

}

static void onmain(struct channel *channel, unsigned int source, void *mdata, unsigned int msize)
{

    list(channel, source, FILE_L0);
    channel_close(channel);

}

static void onfile(struct channel *channel, unsigned int source, void *mdata, unsigned int msize)
{

    file_walk2(FILE_L0, mdata);

}

static void onredirect(struct channel *channel, unsigned int source, void *mdata, unsigned int msize)
{

    struct event_redirect *redirect = mdata;

    channel_setredirect(channel, redirect->type, redirect->mode, redirect->id, source);

}

static void oninit(struct channel *channel)
{

    file_duplicate(FILE_L0, FILE_PW);

}

static void onexit(struct channel *channel)
{

}

void main(void)
{

    struct channel channel;

    channel_init(&channel);
    channel_setsignal(&channel, EVENT_MAIN, onmain);
    channel_setsignal(&channel, EVENT_FILE, onfile);
    channel_setsignal(&channel, EVENT_REDIRECT, onredirect);
    channel_listen2(&channel, oninit, onexit);

}

