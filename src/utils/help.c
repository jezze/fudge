#include <fudge.h>
#include <abi.h>

static void ondata(unsigned int source, void *mdata, unsigned int msize)
{

    channel_send_buffer(0, source, EVENT_DATA, msize, mdata);

}

static void onmain(unsigned int source, void *mdata, unsigned int msize)
{

    unsigned int channel = fs_spawn(0, option_getstring("echo"));

    if (channel)
    {

        channel_route(EVENT_DATA, source);
        channel_send(0, channel, EVENT_MAIN);
        channel_send_fmt0(0, channel, EVENT_PATH, "initrd:data/help.txt\\0");
        channel_send(0, channel, EVENT_END);
        channel_wait(0, channel, EVENT_DONE);

    }

}

void init(void)
{

    option_add("echo", "initrd:bin/echo");
    channel_bind(EVENT_DATA, ondata);
    channel_bind(EVENT_MAIN, onmain);

}

