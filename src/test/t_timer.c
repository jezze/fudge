#include <fudge.h>
#include <abi.h>

static unsigned int counter = 1;

static void onmain(unsigned int source, void *mdata, unsigned int msize)
{

    struct message message;

    if (!file_walk2(FILE_L0, option_getstring("timer")))
        channel_error("Could not find timer device");

    if (!file_walk(FILE_L1, FILE_L0, "event100"))
        channel_error("Could not find timer device event100");

    file_link(FILE_L1);

    while (channel_kpollevent(EVENT_TIMERTICK, &message))
    {

        channel_sendfmt1(EVENT_DATA, "Tick: %u second(s)\n", &counter);

        counter++;

    }

    file_unlink(FILE_L1);
    channel_close();

}

void init(void)
{

    option_add("timer", "system:timer/if:0");
    channel_bind(EVENT_MAIN, onmain);

}

