#include <fudge.h>
#include <abi.h>

static void onmain(struct channel *channel, unsigned int source, void *mdata, unsigned int msize)
{

    char *data = "Hello world!\n";

    channel_place(channel, source, EVENT_DATA, ascii_length(data), data);
    channel_close(channel);

}

void main(void)
{

    struct channel channel;

    channel_init(&channel);
    channel_setsignal(&channel, EVENT_MAIN, onmain);
    channel_listen(&channel, 0, 0);

}

