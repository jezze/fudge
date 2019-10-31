#include <fudge.h>
#include <abi.h>

static void ondone(struct channel *channel, void *mdata, unsigned int msize)
{

    channel_close(channel);

}

static void onempty(struct channel *channel, void *mdata, unsigned int msize)
{

    unsigned int id;

    if (!file_walk2(FILE_CP, "/bin/echo"))
        return;

    id = call_spawn(FILE_CP);

    if (!id)
        return;

    channel_forward(channel, EVENT_FILE);
    channel_append(channel, 15, "/data/help.txt");
    channel_place(channel, id);
    channel_forward(channel, EVENT_DONE);
    channel_place(channel, id);

}

void main(void)
{

    struct channel channel;

    channel_init(&channel);
    channel_setsignal(&channel, EVENT_DONE, ondone);
    channel_setsignal(&channel, EVENT_EMPTY, onempty);
    channel_listen(&channel);

}

