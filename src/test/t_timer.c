#include <fudge.h>
#include <abi.h>

static unsigned int counter;

static void onmain(struct channel *channel, unsigned int source, void *mdata, unsigned int msize)
{

    struct message_header header;
    struct message_data data;

    file_link(FILE_G0);

    while (channel_pollsource(channel, 0, &header, &data))
    {

        if (header.event == EVENT_TIMERTICK)
        {

            unsigned int offset = 0;

            offset = message_appendstring(&data, "Tick: ", offset);
            offset = message_appendvalue(&data, counter++, 10, 0, offset);
            offset = message_appendstring(&data, "\n", offset);

            channel_place(channel, source, EVENT_DATA, offset, &data);


        }

    }

    file_unlink(FILE_G0);
    channel_close(channel, source);

}

void init(struct channel *channel)
{

    if (!file_walk2(FILE_G0, "/system/timer/if:0/event"))
        return;

    channel_setcallback(channel, EVENT_MAIN, onmain);

}

