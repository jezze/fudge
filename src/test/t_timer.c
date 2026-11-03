#include <fudge.h>
#include <abi.h>

static unsigned int output;
static unsigned int counter = 1;

static void ontimertick(struct message *message)
{

    channel_send_fmt(0, output, MESSAGE_TYPE_DATA, "Tick: %u second(s)\n", &counter);

    counter++;

}

static void onmain(struct message *message)
{

    unsigned int timer = channel_lookup(option_getstring("timer-service"));

    if (timer)
    {

        output = message->source;

        channel_send(0, timer, MESSAGE_TYPE_LINK, 0, 0);
        channel_hold(0);
        channel_send(0, timer, MESSAGE_TYPE_UNLINK, 0, 0);

    }

}

void init(void)
{

    option_add("timer-service", "timer");
    channel_bind(MESSAGE_TYPE_MAIN, onmain);
    channel_bind(MESSAGE_TYPE_TIMERTICK, ontimertick);

}

