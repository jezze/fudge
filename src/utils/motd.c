#include <fudge.h>
#include <abi.h>

static void ondata(unsigned int source, void *mdata, unsigned int msize)
{

    channel_send_buffer(source, EVENT_DATA, msize, mdata);

}

static void onmain(unsigned int source, void *mdata, unsigned int msize)
{

    unsigned int channel = fs_spawn(option_getstring("echo"));

    if (channel)
    {

        channel_route(EVENT_DATA, source);
        channel_send(channel, EVENT_MAIN);
        channel_send_fmt0(channel, EVENT_PATH, "initrd:data/motd.txt\\0");
        channel_send(channel, EVENT_END);
        channel_wait(EVENT_TERMRESPONSE);

    }

}

void init(void)
{

    option_add("echo", "initrd:bin/echo");
    channel_bind(EVENT_DATA, ondata);
    channel_bind(EVENT_MAIN, onmain);

}

