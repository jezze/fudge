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
            file_create(FILE_L1, FILE_L0, buffer + offset);
        else
            channel_error("Parent directory not found");

    }

    else
    {

        file_create(FILE_L1, FILE_PW, mdata);

    }

}

void init(void)
{

    channel_bind(EVENT_PATH, onpath);

}

