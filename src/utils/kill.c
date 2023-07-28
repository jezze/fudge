#include <fudge.h>
#include <abi.h>

static void onmain(unsigned int source, void *mdata, unsigned int msize)
{

    unsigned int task = option_getdecimal("task");

    if (task)
        call_kill(task);

}

void init(void)
{

    option_add("task", "");
    channel_bind(EVENT_MAIN, onmain);

}

