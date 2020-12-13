#include <fudge.h>
#include <abi.h>

static void onargument(struct channel *channel, unsigned int source, void *mdata, unsigned int msize)
{

    if (file_walk2(FILE_L0, mdata))
        channel_place(channel, EVENT_ARGUMENT, msize, mdata);

}

void init(struct channel *channel)
{

    channel_setcallback(channel, EVENT_ARGUMENT, onargument);

}

