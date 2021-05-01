#include <fudge.h>
#include <abi.h>

static void onpath(struct channel *channel, unsigned int source, void *mdata, unsigned int msize)
{

    if (file_walk2(FILE_L0, mdata))
        call_destroy(FILE_L0);

}

void init(struct channel *channel)
{

    channel_setcallback(channel, EVENT_PATH, onpath);

}

