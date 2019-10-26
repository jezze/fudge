#include <fudge.h>
#include <abi.h>

static void ondone(struct channel *channel, void *mdata, unsigned int msize)
{

    unsigned int id = channel_reply(channel, EVENT_DATA);

    channel_appendstring(channel, "Hello world!\n");
    channel_place(channel, id);
    channel_close(channel);

}

void main(void)
{

    struct channel channel;

    channel_init(&channel);
    channel_setsignal(&channel, EVENT_DONE, ondone);
    channel_listen(&channel);

}

