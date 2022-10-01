#include <fudge.h>
#include <abi.h>

static void onmain(unsigned int source, void *mdata, unsigned int msize)
{

    if (file_walk2(FILE_L0, option_getstring("log")))
    {

        char buffer[BUFFER_SIZE];
        unsigned int count;

        file_link(FILE_L0);

        while ((count = channel_read(buffer, BUFFER_SIZE)))
            channel_sendbuffer(EVENT_DATA, count, buffer);

        file_unlink(FILE_L0);

    }

    else
    {

        channel_error("Log not found");

    }

    channel_close();

}

void init(void)
{

    option_add("log", "system:log/messages");
    channel_bind(EVENT_MAIN, onmain);

}

