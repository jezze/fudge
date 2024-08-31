#include <fudge.h>
#include <abi.h>

static void onpath(unsigned int source, void *mdata, unsigned int msize)
{

}

void init(void)
{

    channel_bind(EVENT_PATH, onpath);

}

