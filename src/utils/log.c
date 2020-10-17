#include <fudge.h>
#include <abi.h>

static void ondata(struct channel *channel, unsigned int source, void *mdata, unsigned int msize)
{

    channel_place3(channel, source, EVENT_DATA, msize, mdata);

}

static void onredirect(struct channel *channel, unsigned int source, void *mdata, unsigned int msize)
{

    struct event_redirect *redirect = mdata;

    channel_setredirect(channel, redirect->type, redirect->mode, redirect->id, source);

}

static void oninit(struct channel *channel)
{

    if (!file_walk2(FILE_G0, "/system/log/messages"))
        return;

    file_open(FILE_G0);

}

static void onexit(struct channel *channel)
{

    file_close(FILE_G0);

}

void main(void)
{

    struct channel channel;

    channel_init(&channel);
    channel_setsignal(&channel, EVENT_DATA, ondata);
    channel_setsignal(&channel, EVENT_REDIRECT, onredirect);
    channel_listen2(&channel, oninit, onexit);

}

