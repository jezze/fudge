#include <fudge.h>
#include <abi.h>

static void onmain(struct channel *channel, unsigned int source, void *mdata, unsigned int msize)
{

    file_mount("/system", 2000);
    channel_close(channel);

}

void init(struct channel *channel)
{

    channel_setsignal(channel, EVENT_MAIN, onmain);

}

