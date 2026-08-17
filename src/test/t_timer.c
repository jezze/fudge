#include <fudge.h>
#include <abi.h>

static unsigned int counter = 1;

static void onmain(unsigned int source, void *mdata, unsigned int msize)
{

    unsigned int timer = channel_lookup(option_getstring("timer-service"));

    if (timer)
    {

        struct message message;
        char data[MESSAGE_SIZE];

        channel_send(0, timer, EVENT_LINK);

        while (channel_poll(0, timer, EVENT_TIMERTICK, &message, MESSAGE_SIZE, data))
        {

            channel_send_fmt1(0, source, EVENT_DATA, "Tick: %u second(s)\n", &counter);

            counter++;

        }

        channel_send(0, timer, EVENT_UNLINK);

    }

}

void init(void)
{

    option_add("timer-service", "timer3");
    channel_bind(EVENT_MAIN, onmain);

    while (channel_process(0));

}

