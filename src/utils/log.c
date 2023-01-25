#include <fudge.h>
#include <abi.h>

static void onmain(unsigned int source, void *mdata, unsigned int msize)
{

    struct message_header header;
    struct message_data data;

    if (!file_walk2(FILE_L0, option_getstring("log")))
        channel_error("Log not found");

    file_link(FILE_L0);

    while (channel_read(EVENT_DATA, &header, &data))
        channel_sendbuffer(EVENT_DATA, message_datasize(&header), &data);

    file_unlink(FILE_L0);
    channel_close();

}

void init(void)
{

    option_add("log", "system:log/messages");
    channel_bind(EVENT_MAIN, onmain);

}

