#include <fudge.h>
#include <abi.h>

static unsigned int ticks;
static unsigned int counter;

static void onmain(struct channel *channel, unsigned int source, void *mdata, unsigned int msize)
{

    struct message_header header;
    struct message_data data;

    file_link(FILE_G0);

    while (channel_polldescriptorevent(channel, FILE_G0, EVENT_TIMERTICK, &header, &data))
    {

        ticks++;

        if (ticks % 6)
        {

            unsigned int offset = 0;

            offset = message_putstring(&data, "Tick: ", offset);
            offset = message_putvalue(&data, counter * 60, 10, 0, offset);
            offset = message_putstring(&data, "ms\n", offset);

            channel_place(channel, source, EVENT_DATA, offset, &data);

            counter++;

        }

    }

    file_unlink(FILE_G0);
    channel_close(channel, source);

}

void init(struct channel *channel)
{

    if (!file_walk2(FILE_G0, "/system/timer/if:0/event10"))
        return;

    channel_setcallback(channel, EVENT_MAIN, onmain);

}

