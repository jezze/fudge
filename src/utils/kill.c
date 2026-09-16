#include <fudge.h>
#include <abi.h>

static void onmain(struct message *message)
{

    unsigned int task = option_getdecimal("task");

    if (task)
        call_kill(task);

}

void init(void)
{

    option_add("task", "");
    channel_bind(EVENT_MAIN, onmain);

    while (channel_process(0));

}

