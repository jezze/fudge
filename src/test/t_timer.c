#include <fudge.h>
#include <abi.h>

static unsigned int output;
static unsigned int counter = 1;

static void ontimertick(struct message *message)
{

    channel_send_fmt(0, output, EVENT_DATA, "Tick: %u second(s)\n", &counter);

    counter++;

}

static void onmain(struct message *message)
{

    unsigned int timer = channel_lookup(option_getstring("timer-service"));

    if (timer)
    {

        output = message->source;

        channel_send(0, timer, EVENT_LINK, 0, 0);
        channel_hold(0);
        channel_send(0, timer, EVENT_UNLINK, 0, 0);

    }

}

void init(void)
{

    option_add("timer-service", "timer");
    channel_bind(EVENT_MAIN, onmain);
    channel_bind(EVENT_TIMERTICK, ontimertick);

}

