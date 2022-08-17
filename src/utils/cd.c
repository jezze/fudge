#include <fudge.h>
#include <abi.h>

static void onpath(unsigned int source, void *mdata, unsigned int msize)
{

    if (file_walk2(FILE_L0, mdata))
        channel_sendbuffer(EVENT_PATH, msize, mdata);
    else
        channel_error("Directory not found");

}

void init(void)
{

    channel_bind(EVENT_PATH, onpath);

}

