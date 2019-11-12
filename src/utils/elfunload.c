#include <fudge.h>
#include <abi.h>

static void ondone(struct channel *channel, unsigned int source, void *mdata, unsigned int msize)
{

    channel_close(channel);

}

static void onfile(struct channel *channel, unsigned int source, void *mdata, unsigned int msize)
{

    if (!file_walk2(FILE_G0, mdata))
        return;

    file_open(FILE_G0);
    call_unload(FILE_G0);
    file_close(FILE_G0);

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
    channel_setsignal(&channel, EVENT_FILE, onfile);
    channel_setsignal(&channel, EVENT_REDIRECT, onredirect);
    channel_listen(&channel);

}

