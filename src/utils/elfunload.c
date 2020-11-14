#include <fudge.h>
#include <abi.h>

static void onfile(struct channel *channel, unsigned int source, void *mdata, unsigned int msize)
{

    if (file_walk2(FILE_G0, mdata))
    {

        file_open(FILE_G0);
        call_unload(FILE_G0);
        file_close(FILE_G0);

    }

}

void init(struct channel *channel)
{

    channel_setsignal(channel, EVENT_FILE, onfile);

}

