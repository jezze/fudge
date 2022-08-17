#include <fudge.h>
#include <abi.h>

static void onmain(unsigned int source, void *mdata, unsigned int msize)
{

    char buffer[BUFFER_SIZE];
    unsigned int count;

    file_link(FILE_G0);

    while ((count = channel_read(buffer, BUFFER_SIZE)))
        channel_sendbuffer(EVENT_DATA, count, buffer);

    file_unlink(FILE_G0);
    channel_close();

}

void init(void)
{

    if (!file_walk2(FILE_G0, "system:log/messages"))
        channel_error("Could not find system log");

    channel_bind(EVENT_MAIN, onmain);

}

