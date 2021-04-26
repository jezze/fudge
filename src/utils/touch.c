#include <fudge.h>
#include <abi.h>

static void onfile(struct channel *channel, unsigned int source, void *mdata, unsigned int msize)
{

    file_create(FILE_L0, FILE_PW, mdata);

}

void init(struct channel *channel)
{

    channel_setcallback(channel, EVENT_FILE, onfile);

}

