#include <fudge.h>
#include <abi.h>

static unsigned int ticks;
static unsigned int counter;

static void onmain(unsigned int source, void *mdata, unsigned int msize)
{

    struct message message;

    file_link(FILE_G0);

    while (channel_polldescriptorevent(FILE_G0, EVENT_TIMERTICK, &message))
    {

        ticks++;

        if (ticks % 6)
        {

            unsigned int offset = 0;

            offset = message_putstring(&message, "Tick: ", offset);
            offset = message_putvalue(&message, counter * 60, 10, 0, offset);
            offset = message_putstring(&message, "ms\n", offset);

            channel_reply(EVENT_DATA, offset, message.data.buffer);

            counter++;

        }

    }

    file_unlink(FILE_G0);
    channel_close();

}

void init(void)
{

    if (!file_walk2(FILE_G0, "system:timer/if:0/event10"))
        return;

    channel_setcallback(EVENT_MAIN, onmain);

}

