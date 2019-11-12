#include <fudge.h>
#include <abi.h>

static void ondone(struct channel *channel, unsigned int source, void *mdata, unsigned int msize)
{

    channel_close(channel);

}

static void onempty(struct channel *channel, unsigned int source, void *mdata, unsigned int msize)
{

    channel_request(channel, EVENT_DATA);
    channel_appendstring(channel, "Hello world!\n");
    channel_place(channel, source);

}

static void onredirect(struct channel *channel, unsigned int source, void *mdata, unsigned int msize)
{

    struct event_redirect *redirect = mdata;

    channel_setredirect(channel, redirect->type, redirect->id, source);

}

void main(void)
{

    struct channel channel;

    channel_init(&channel);
    channel_setsignal(&channel, EVENT_DONE, ondone);
    channel_setsignal(&channel, EVENT_EMPTY, onempty);
    channel_setsignal(&channel, EVENT_REDIRECT, onredirect);
    channel_listen(&channel);

}

