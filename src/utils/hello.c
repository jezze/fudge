#include <fudge.h>
#include <abi.h>

static void onstop(struct channel *channel, void *mdata, unsigned int msize)
{

    unsigned int id = channel_reply(channel, EVENT_DATA);

    channel_append(channel, 13, "Hello world!\n");
    channel_place(channel, id);
    channel_exit(channel);

}

void main(void)
{

    struct channel channel;

    channel_init(&channel);
    channel_setsignal(&channel, EVENT_STOP, onstop);
    channel_listen(&channel);

}

