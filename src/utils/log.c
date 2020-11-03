#include <fudge.h>
#include <abi.h>

static void ondata(struct channel *channel, unsigned int source, void *mdata, unsigned int msize)
{

    channel_place(channel, source, EVENT_DATA, msize, mdata);

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
    channel_listen(&channel, oninit, onexit);

}

