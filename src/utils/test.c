#include <fudge.h>
#include <abi.h>

static unsigned int from;

static void onempty(struct channel *channel, void *mdata, unsigned int msize)
{

    from = channel->i.source;

}

static void ontimertick(struct channel *channel, void *mdata, unsigned int msize)
{

    if (from)
    {

        channel_request(channel, EVENT_DATA);
        channel_appendstring(channel, "HEJ!\n");
        channel_place(channel, from);

    }

}

void main(void)
{

    struct channel channel;

    if (!file_walk2(FILE_G0, "/system/timer/if:0/event"))
        return;

    if (!file_walk2(FILE_G1, "/system/timer/if:0/ctrl"))
        return;

    file_open(FILE_G0);
    channel_init(&channel);
    channel_setsignal(&channel, EVENT_EMPTY, onempty);
    channel_setsignal(&channel, EVENT_TIMERTICK, ontimertick);
    channel_listen(&channel);
    file_close(FILE_G0);

}

