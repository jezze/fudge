#include <fudge.h>
#include <abi.h>

static void ondata(unsigned int source, void *mdata, unsigned int msize)
{

    unsigned int service = fsp_auth(option_getstring("output"));

    if (service)
    {

        unsigned int id = fsp_walk(service, 0, option_getstring("output"));

        if (id)
            fsp_write(service, id, mdata, msize, 0);

    }

}

static void onmain(unsigned int source, void *mdata, unsigned int msize)
{

    call_announce(option_getdecimal("listen"));

    while (channel_process());

}

void init(void)
{

    option_add("listen", "101");
    option_add("output", "system:console/if.0/data");
    channel_bind(EVENT_DATA, ondata);
    channel_bind(EVENT_MAIN, onmain);

}

