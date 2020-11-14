#include <fudge.h>
#include <abi.h>

static void onmain(struct channel *channel, unsigned int source, void *mdata, unsigned int msize)
{

    file_open(FILE_L0);
    file_writeall(FILE_L0, "1", 1);
    file_close(FILE_L0);
    channel_close(channel);

}

void init(struct channel *channel)
{

    if (!file_walk2(FILE_L0, "/system/reset"))
        return;

    channel_setsignal(channel, EVENT_MAIN, onmain);

}

