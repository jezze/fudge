#include <fudge.h>
#include <abi.h>

static void ondata(unsigned int source, void *mdata, unsigned int msize)
{

    channel_sendbuffer(CHANNEL_DEFAULT, EVENT_DATA, msize, mdata);

}

static void onpath(unsigned int source, void *mdata, unsigned int msize)
{

    if (file_walk2(FILE_L0, mdata))
    {

        char buffer[MESSAGE_SIZE];
        unsigned int count;

        while ((count = file_read(FILE_L0, buffer, MESSAGE_SIZE)))
            channel_sendbuffer(CHANNEL_DEFAULT, EVENT_DATA, count, buffer);

    }

    else
    {

        channel_sendfmt1(CHANNEL_DEFAULT, EVENT_ERROR, "Path not found: %s\n", mdata);

    }

}

void init(void)
{

    channel_bind(EVENT_DATA, ondata);
    channel_bind(EVENT_PATH, onpath);

}

