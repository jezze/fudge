#include <fudge.h>
#include <abi.h>

static void onmain(unsigned int source, void *mdata, unsigned int msize)
{

    struct message_data data;
    unsigned int count;

    if (!file_walk2(FILE_L0, option_getstring("log")))
        channel_error("Log not found");

    file_link(FILE_L0);

    while ((count = channel_read(&data)))
        channel_sendbuffer(EVENT_DATA, count, &data);

    file_unlink(FILE_L0);
    channel_close();

}

void init(void)
{

    option_add("log", "system:log/messages");
    channel_bind(EVENT_MAIN, onmain);

}

