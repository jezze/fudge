#include <fudge.h>
#include <abi.h>

static void ondata(unsigned int source, void *mdata, unsigned int msize)
{

    channel_send(0, source, EVENT_DATA, msize, mdata);

}

static void onmain(unsigned int source, void *mdata, unsigned int msize)
{

    unsigned int target = fs_spawn(1, "initrd:bin/echo");

    if (target)
    {

        channel_route(EVENT_DATA, source);
        channel_send(1, target, EVENT_MAIN, 0, 0);
        channel_send_fmt0(1, target, EVENT_PATH, "initrd:data/motd.txt\\0");
        channel_send(1, target, EVENT_TERM, 0, 0);
        channel_wait(1, target, EVENT_DONE, 0, 0);

    }

}

void init(void)
{

    channel_bind(EVENT_DATA, ondata);
    channel_bind(EVENT_MAIN, onmain);

    while (channel_process(0));

}

