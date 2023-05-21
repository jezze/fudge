#include <fudge.h>
#include <abi.h>

static void onpath(unsigned int source, void *mdata, unsigned int msize)
{

    if (call_walk_absolute(FILE_L0, mdata))
        call_destroy(FILE_L0);
    else
        channel_send_fmt1(CHANNEL_DEFAULT, EVENT_ERROR, "Path not found: %s\n", mdata);

}

void init(void)
{

    channel_bind(EVENT_PATH, onpath);

}

