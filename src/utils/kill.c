#include <fudge.h>
#include <abi.h>

static void onmain(struct message *message)
{

    unsigned int target = option_getdecimal("target");

    if (target)
        call_kill(target);

}

void init(void)
{

    option_add("target", "");
    channel_bind(EVENT_MAIN, onmain);

    while (channel_process(0));

}

