#include <fudge.h>
#include <abi.h>

static unsigned int counter = 1;

static void onmain(unsigned int source, void *mdata, unsigned int msize)
{

    struct message message;

    file_link(FILE_G0);

    while (channel_kpollevent(EVENT_TIMERTICK, &message))
    {

        message_init(&message, EVENT_DATA);
        message_putstring(&message, "Tick: ");
        message_putvalue(&message, counter, 10, 0);
        message_putstring(&message, " second\n");
        channel_sendmessage(&message);

        counter++;

    }

    file_unlink(FILE_G0);
    channel_close();

}

void init(void)
{

    file_walk2(FILE_G0, "system:timer/if:0/event100");
    channel_bind(EVENT_MAIN, onmain);

}

