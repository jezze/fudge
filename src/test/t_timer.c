#include <fudge.h>
#include <abi.h>

static void onredirect(struct channel *channel, unsigned int source, void *mdata, unsigned int msize)
{

    struct event_redirect *redirect = mdata;

    channel_setredirect(channel, redirect->type, redirect->mode, redirect->id, source);

}

static void ontimertick(struct channel *channel, unsigned int source, void *mdata, unsigned int msize)
{

    union message message;

    channel_header(&message, EVENT_DATA);
    channel_appendstring(&message, "HEJ!\n");
    channel_place(channel, &message, source);

}

static void oninit(struct channel *channel)
{

    if (!file_walk2(FILE_G0, "/system/timer/if:0/event"))
        return;

    if (!file_walk2(FILE_G1, "/system/timer/if:0/ctrl"))
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
    channel_setsignal(&channel, EVENT_REDIRECT, onredirect);
    channel_setsignal(&channel, EVENT_TIMERTICK, ontimertick);
    channel_listen2(&channel, oninit, onexit);

}

