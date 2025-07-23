#include <fudge.h>
#include <abi.h>

static unsigned int counter = 1;

static void onmain(unsigned int source, void *mdata, unsigned int msize)
{

    struct message message;
    char data[MESSAGE_SIZE];

    option_setdecimal("clock-service", lookup(option_getstring("clock-service")));
    channel_send(0, option_getdecimal("timer-service"), EVENT_LINK);

    while (channel_poll(0, option_getdecimal("timer-service"), EVENT_TIMERTICK, &message, MESSAGE_SIZE, data))
    {

        channel_send_fmt1(0, source, EVENT_DATA, "Tick: %u second(s)\n", &counter);

        counter++;

    }

    channel_send(0, option_getdecimal("timer-service"), EVENT_UNLINK);

}

void init(void)
{

    option_add("timer-service", "timer3");
    channel_bind(EVENT_MAIN, onmain);

}

