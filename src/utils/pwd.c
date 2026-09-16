#include <fudge.h>
#include <abi.h>

static void onmain(struct message *message)
{

    channel_send_fmt1(0, message->source, EVENT_DATA, "%s\n", option_getstring("pwd"));

}

void init(void)
{

    channel_bind(EVENT_MAIN, onmain);

    while (channel_process(0));

}

