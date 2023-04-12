#include <fudge.h>
#include <abi.h>

static void onpath(unsigned int source, void *mdata, unsigned int msize)
{

    unsigned int offset = buffer_lastbyte(mdata, msize, '/');

    if (offset)
    {

        char *buffer = mdata;

        buffer[offset - 1] = '\0';

        if (file_walk2(FILE_L0, buffer))
        {

            if (!file_create(FILE_L1, FILE_L0, buffer + offset))
                channel_sendfmt1(CHANNEL_DEFAULT, EVENT_ERROR, "Create path failed: %s\n", mdata);

        }

        else
        {

            channel_sendfmt1(CHANNEL_DEFAULT, EVENT_ERROR, "Path not found: %s\n", mdata);

        }

    }

    else
    {

        if (!file_create(FILE_L1, FILE_PW, mdata))
            channel_sendfmt1(CHANNEL_DEFAULT, EVENT_ERROR, "Create path failed: %s\n", mdata);

    }

}

void init(void)
{

    channel_bind(EVENT_PATH, onpath);

}

