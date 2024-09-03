#include <fudge.h>
#include <abi.h>

static unsigned int counter = 1;

static void onmain(unsigned int source, void *mdata, unsigned int msize)
{

    struct message message;
    char data[MESSAGE_SIZE];

    channel_send(option_getdecimal("timer-service"), EVENT_LINK);

    while (channel_poll(EVENT_TIMERTICK, &message, MESSAGE_SIZE, data))
    {

        channel_send_fmt1(source, EVENT_DATA, "Tick: %u second(s)\n", &counter);

        counter++;

    }

    channel_send(option_getdecimal("timer-service"), EVENT_UNLINK);

}

void init(void)
{

    option_add("timer-service", "412");
    channel_bind(EVENT_MAIN, onmain);

}

