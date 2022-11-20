#include <fudge.h>
#include <abi.h>

static void onmain(unsigned int source, void *mdata, unsigned int msize)
{

    unsigned int id = option_getdecimal("task");

    if (id)
        channel_sendto(id, EVENT_STATUS);

    channel_close();

}

void init(void)
{

    option_add("task", "");
    channel_bind(EVENT_MAIN, onmain);

}

