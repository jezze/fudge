#include <fudge.h>
#include <abi.h>

static void onpath(unsigned int source, void *mdata, unsigned int msize)
{

    if (file_walk2(FILE_G0, mdata))
        call_unload(FILE_G0);

}

void init(void)
{

    channel_bind(EVENT_PATH, onpath);

}

