#include <fudge.h>
#include <abi.h>

static void onmain(unsigned int source, void *mdata, unsigned int msize)
{

    char data[MESSAGE_SIZE];
    unsigned int count;

    if (!file_walk2(FILE_L0, option_getstring("log")))
        channel_error("Log not found");

    file_link(FILE_L0);

    while ((count = channel_read(MESSAGE_SIZE, data)))
        channel_sendbuffer(EVENT_DATA, count, data);

    file_unlink(FILE_L0);
    channel_close();

}

void init(void)
{

    option_add("log", "system:log/messages");
    channel_bind(EVENT_MAIN, onmain);

}

