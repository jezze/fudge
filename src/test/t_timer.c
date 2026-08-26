#include <fudge.h>
#include <abi.h>

static void onmain(unsigned int source, void *mdata, unsigned int msize)
{

    unsigned int timer = channel_lookup(option_getstring("timer-service"));

    if (timer)
    {

        unsigned int counter = 1;

        channel_send(0, timer, EVENT_LINK, 0, 0);

        while (channel_wait(0, timer, EVENT_TIMERTICK, 0, 0))
        {

            channel_send_fmt1(0, source, EVENT_DATA, "Tick: %u second(s)\n", &counter);

            counter++;

        }

        channel_send(0, timer, EVENT_UNLINK, 0, 0);

    }

}

void init(void)
{

    option_add("timer-service", "timer3");
    channel_bind(EVENT_MAIN, onmain);

    while (channel_process(0));

}

