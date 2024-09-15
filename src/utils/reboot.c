#include <fudge.h>
#include <abi.h>

static void onmain(unsigned int source, void *mdata, unsigned int msize)
{

    channel_send(option_getdecimal("reset-service"), EVENT_TERMREQUEST);

}

void init(void)
{

    option_add("reset-service", "107");
    channel_bind(EVENT_MAIN, onmain);

}

