#include <fudge.h>
#include <abi.h>

static void onpath(unsigned int source, void *mdata, unsigned int msize)
{

    if (file_walk2(FILE_L0, mdata))
        channel_sendbuffer(CHANNEL_DEFAULT, EVENT_PATH, msize, mdata);
    else
        channel_sendfmt1(CHANNEL_DEFAULT, EVENT_ERROR, "Path not found: %s\n", mdata);

}

void init(void)
{

    channel_bind(EVENT_PATH, onpath);

}

