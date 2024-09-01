#include <fudge.h>
#include <abi.h>

static unsigned int counter = 1;

static void onmain(unsigned int source, void *mdata, unsigned int msize)
{

    unsigned int timerservice = fsp_auth(option_getstring("timer"));
    unsigned int timerevent = fsp_walk(timerservice, fsp_walk(timerservice, 0, option_getstring("timer")), "event100");
    struct message message;
    char data[MESSAGE_SIZE];

    fsp_link(timerservice, timerevent);

    while (channel_poll(EVENT_TIMERTICK, &message, MESSAGE_SIZE, data))
    {

        channel_send_fmt1(source, EVENT_DATA, "Tick: %u second(s)\n", &counter);

        counter++;

    }

    fsp_unlink(timerservice, timerevent);

}

void init(void)
{

    option_add("timer", "system:timer/if.0");
    channel_bind(EVENT_MAIN, onmain);

}

