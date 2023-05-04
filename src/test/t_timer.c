#include <fudge.h>
#include <abi.h>

static unsigned int counter = 1;

static void onmain(unsigned int source, void *mdata, unsigned int msize)
{

    struct message message;
    char data[MESSAGE_SIZE];

    if (!file_walk2(FILE_L0, option_getstring("timer")))
        PANIC();

    if (!file_walk(FILE_L1, FILE_L0, "event100"))
        PANIC();

    file_link(FILE_L1, 8000);

    while (channel_poll(EVENT_TIMERTICK, &message, data))
    {

        channel_sendfmt1(CHANNEL_DEFAULT, EVENT_DATA, "Tick: %u second(s)\n", &counter);

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

