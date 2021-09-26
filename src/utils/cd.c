#include <fudge.h>
#include <abi.h>

static void onpath(unsigned int source, void *mdata, unsigned int msize)
{

    if (file_walk2(FILE_L0, mdata))
        channel_reply(EVENT_PATH, msize, mdata);

}

void init(void)
{

    channel_bind(EVENT_PATH, onpath);

}

