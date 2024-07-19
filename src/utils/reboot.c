#include <fudge.h>
#include <abi.h>

static void onmain(unsigned int source, void *mdata, unsigned int msize)
{

    if (call_walk_absolute(FILE_L0, "system:reset"))
        call_notify(FILE_L0, EVENT_TERMREQUEST, 0, 0);
    else
        channel_send_fmt1(source, EVENT_ERROR, "Path not found: %s\n", "system:reset");

}

void init(void)
{

    channel_bind(EVENT_MAIN, onmain);

}

