#include <fudge.h>
#include <abi.h>

static void onopen(struct channel *channel, void *mdata, unsigned int msize)
{

    unsigned int id = channel_reply(channel, EVENT_DATA);

    channel_appendstring(channel, "Hello world!\n");
    channel_place(channel, id);

}

static void onclose(struct channel *channel, void *mdata, unsigned int msize)
{

    channel_close(channel);

}

void main(void)
{

    struct channel channel;

    channel_init(&channel);
    channel_setsignal(&channel, EVENT_OPEN, onopen);
    channel_setsignal(&channel, EVENT_CLOSE, onclose);
    channel_listen(&channel);

}

