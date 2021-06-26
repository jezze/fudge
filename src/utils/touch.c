#include <fudge.h>
#include <abi.h>

static void onpath(struct channel *channel, unsigned int source, void *mdata, unsigned int msize)
{

    unsigned int lb = buffer_findlastbyte(mdata, msize, '/');

    if (lb)
    {

        char *buffer = mdata;

        buffer[lb] = '\0';

        if (!file_walk2(FILE_L0, buffer))
            return;

        file_create(FILE_L1, FILE_L0, buffer + lb + 1);

    }

    else
    {

        file_create(FILE_L1, FILE_PW, mdata);

    }

}

void init(struct channel *channel)
{

    channel_setcallback(channel, EVENT_PATH, onpath);

}

