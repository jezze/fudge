#include <fudge.h>
#include <abi.h>

static void ondata(unsigned int source, void *mdata, unsigned int msize)
{

    channel_send_buffer(0, source, EVENT_DATA, msize, mdata);

}

static void onmain(unsigned int source, void *mdata, unsigned int msize)
{

    unsigned int target = fs_spawn(1, option_getstring("echo"));

    if (target)
    {

        channel_route(EVENT_DATA, source);
        channel_send(1, target, EVENT_MAIN);
        channel_send_fmt0(1, target, EVENT_PATH, "initrd:data/motd.txt\\0");
        channel_send(1, target, EVENT_END);
        channel_wait(1, target, EVENT_DONE);

    }

}

void init(void)
{

    option_add("echo", "initrd:bin/echo");
    channel_bind(EVENT_DATA, ondata);
    channel_bind(EVENT_MAIN, onmain);

}

