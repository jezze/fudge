#include <fudge.h>
#include <abi.h>

static void onmain(struct channel *channel, unsigned int source, void *mdata, unsigned int msize)
{

    char *data = "Hello world!\n";

    channel_place(channel, source, EVENT_DATA, ascii_length(data), data);
    channel_close(channel, source);

}

void init(struct channel *channel)
{

    channel_setcallback(channel, EVENT_MAIN, onmain);

}

