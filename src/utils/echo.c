#include <fudge.h>
#include <abi.h>

static void ondata(unsigned int source, void *mdata, unsigned int msize)
{

    channel_replybuffer(EVENT_DATA, msize, mdata);

}

static void onpath(unsigned int source, void *mdata, unsigned int msize)
{

    if (file_walk2(FILE_L0, mdata))
    {

        char buffer[BUFFER_SIZE];
        unsigned int count;

        file_reset(FILE_L0);

        while ((count = file_read(FILE_L0, buffer, BUFFER_SIZE)))
            channel_replybuffer(EVENT_DATA, count, buffer);

    }

}

void init(void)
{

    channel_bind(EVENT_DATA, ondata);
    channel_bind(EVENT_PATH, onpath);

}

