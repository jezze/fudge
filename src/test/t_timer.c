#include <fudge.h>
#include <abi.h>

static unsigned int counter = 1;

static void onmain(unsigned int source, void *mdata, unsigned int msize)
{

    struct message message;
    char data[MESSAGE_SIZE];

    if (!call_walk_absolute(FILE_L0, option_getstring("timer")))
        PANIC();

    if (!call_walk_relative(FILE_L1, FILE_L0, "event100"))
        PANIC();

    call_link(FILE_L1, 8000);

    while (channel_poll_any(EVENT_TIMERTICK, &message, data))
    {

        channel_send_fmt1(CHANNEL_DEFAULT, EVENT_DATA, "Tick: %u second(s)\n", &counter);

        counter++;

    }

    call_unlink(FILE_L1);

}

void init(void)
{

    option_add("timer", "system:timer/if.0");
    channel_bind(EVENT_MAIN, onmain);

}

