#include <fudge.h>
#include <abi.h>

static void onmain(struct channel *channel, unsigned int source, void *mdata, unsigned int msize)
{

    struct message_header header;
    struct message_data data;

    while (channel_pollsource(channel, 0, &header, &data))
    {

        if (header.type == EVENT_TIMERTICK)
        {

            char *data = "HEJ\n";

            channel_place(channel, source, EVENT_DATA, ascii_length(data), data);


        }

    }

    channel_close(channel);

}

static void oninit(struct channel *channel)
{

    if (!file_walk2(FILE_G0, "/system/timer/if:0/event"))
        return;

    if (!file_walk2(FILE_G1, "/system/timer/if:0/ctrl"))
        return;

    file_open(FILE_G0);

}

static void onexit(struct channel *channel)
{

    file_close(FILE_G0);

}

void main(void)
{

    struct channel channel;

    channel_init(&channel);
    channel_setsignal(&channel, EVENT_MAIN, onmain);
    channel_listen(&channel, oninit, onexit);

}

