#include <fudge.h>
#include <abi.h>

static void onmain(struct message *message)
{

    unsigned int timestamp = system_unixtime(option_getstring("clock-service"));

    if (timestamp)
        channel_send_fmt(0, message->source, EVENT_DATA, "%u\n", &timestamp);

}

void init(void)
{

    option_add("clock-service", "clock");
    channel_bind(EVENT_MAIN, onmain);

}

