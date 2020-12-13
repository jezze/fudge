#include <fudge.h>
#include <abi.h>

static void onmain(struct channel *channel, unsigned int source, void *mdata, unsigned int msize)
{

    struct message_data data;

    channel_place(channel, EVENT_DATA, message_putstring(&data, "Hello World!\n", 0), &data);
    channel_close(channel);

}

void init(struct channel *channel)
{

    channel_setcallback(channel, EVENT_MAIN, onmain);

}

