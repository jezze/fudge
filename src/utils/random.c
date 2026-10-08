#include <fudge.h>
#include <abi.h>

static void onmain(struct message *message)
{

    struct mtwist_state state;
    unsigned int value;

    mtwist_seed1(&state, system_unixtime(option_getstring("clock-service")));

    value = mtwist_rand(&state);

    channel_send_fmt(0, message->source, EVENT_DATA, "%u\n", &value);

}

void init(void)
{

    option_add("clock-service", "clock");
    channel_bind(EVENT_MAIN, onmain);

}

