#include <fudge.h>
#include <abi.h>

static unsigned int ticks;
static unsigned int counter;

static void onmain(unsigned int source, void *mdata, unsigned int msize)
{

    struct message message;

    file_link(FILE_G0);

    while (channel_polleventsystem(EVENT_TIMERTICK, &message))
    {

        ticks++;

        if (ticks % 6)
        {

            message_init(&message, EVENT_DATA);
            message_putstring(&message, "Tick: ");
            message_putvalue(&message, counter * 60, 10, 0);
            message_putstring(&message, "ms\n");
            channel_sendmessage(&message);

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

    channel_bind(EVENT_MAIN, onmain);

}

