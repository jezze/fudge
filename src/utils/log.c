#include <fudge.h>
#include <abi.h>

static void onmain(struct channel *channel, unsigned int source, void *mdata, unsigned int msize)
{

    struct message_data data;
    unsigned int count;

    file_link(FILE_G0);

    while ((count = channel_readdescriptor(channel, FILE_G0, data.buffer)))
        channel_reply(channel, EVENT_DATA, count, data.buffer);

    file_unlink(FILE_G0);
    channel_close(channel);

}

void init(struct channel *channel)
{

    if (!file_walk2(FILE_G0, "system:log/messages"))
        return;

    channel_setcallback(channel, EVENT_MAIN, onmain);

}

