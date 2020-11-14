#include <fudge.h>
#include <abi.h>

static void onmain(struct channel *channel, unsigned int source, void *mdata, unsigned int msize)
{

    struct message_header header;
    struct message_data data;

    file_open(FILE_G0);

    while (channel_pollsource(channel, 0, &header, &data))
        channel_place(channel, source, EVENT_DATA, message_datasize(&header), &data);

    file_close(FILE_G0);
    channel_close(channel);

}

void init(struct channel *channel)
{

    if (!file_walk2(FILE_G0, "/system/log/messages"))
        return;

    channel_setsignal(channel, EVENT_MAIN, onmain);

}

