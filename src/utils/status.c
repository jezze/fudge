#include <fudge.h>
#include <abi.h>

static void ondata(unsigned int source, void *mdata, unsigned int msize)
{

    channel_send_buffer(CHANNEL_DEFAULT, EVENT_DATA, msize, mdata);

}

static void onmain(unsigned int source, void *mdata, unsigned int msize)
{

    unsigned int channel = option_getdecimal("channel");

    if (channel)
        channel_send(channel, EVENT_STATUS);

}

void init(void)
{

    option_add("channel", "");
    channel_bind(EVENT_DATA, ondata);
    channel_bind(EVENT_MAIN, onmain);

}

