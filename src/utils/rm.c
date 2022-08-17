#include <fudge.h>
#include <abi.h>

static void onpath(unsigned int source, void *mdata, unsigned int msize)
{

    if (file_walk2(FILE_L0, mdata))
        call_destroy(FILE_L0);
    else
        channel_error("File not found");

}

void init(void)
{

    channel_bind(EVENT_PATH, onpath);

}

