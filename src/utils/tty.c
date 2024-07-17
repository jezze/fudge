#include <fudge.h>
#include <abi.h>

static void ondata(unsigned int source, void *mdata, unsigned int msize)
{

    call_notify(FILE_G0, EVENT_DATA, msize, mdata);

}

static void onmain(unsigned int source, void *mdata, unsigned int msize)
{

    call_walk_absolute(FILE_G0, option_getstring("output"));
    call_announce(option_getdecimal("listen"));

}

void init(void)
{

    option_add("listen", "101");
    option_add("output", "system:console/if.0/data");
    channel_autoclose(EVENT_END, 0);
    channel_bind(EVENT_DATA, ondata);
    channel_bind(EVENT_MAIN, onmain);

}

