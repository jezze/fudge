#include <fudge.h>
#include <abi.h>

static void onmain(struct channel *channel, unsigned int source, void *mdata, unsigned int msize)
{

    struct message_data data;

    channel_place(channel, source, EVENT_DATA, message_putstring(&data, "Hello World!\n", 0), &data);
    channel_close(channel, source);

}

void init(struct channel *channel)
{

    channel_setcallback(channel, EVENT_MAIN, onmain);

}

