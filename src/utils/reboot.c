#include <fudge.h>
#include <abi.h>

static void onmain(unsigned int source, void *mdata, unsigned int msize)
{

    if (file_walk2(FILE_L0, "system:reset"))
        file_notify(FILE_L0, EVENT_TERM, 0, 0);
    else
        channel_sendfmt1(CHANNEL_DEFAULT, EVENT_ERROR, "Path not found: %s\n", "system:reset");

    channel_close();

}

void init(void)
{

    channel_bind(EVENT_MAIN, onmain);

}

