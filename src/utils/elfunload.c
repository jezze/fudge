#include <fudge.h>
#include <abi.h>

static void ondone(struct channel *channel, void *mdata, unsigned int msize)
{

    channel_close(channel);

}

static void onfile(struct channel *channel, void *mdata, unsigned int msize)
{

    if (!file_walk2(FILE_G0, mdata))
        return;

    file_open(FILE_G0);
    call_unload(FILE_G0);
    file_close(FILE_G0);

}

void main(void)
{

    struct channel channel;

    channel_init(&channel);
    channel_setsignal(&channel, EVENT_DONE, ondone);
    channel_setsignal(&channel, EVENT_FILE, onfile);
    channel_listen(&channel);

}

