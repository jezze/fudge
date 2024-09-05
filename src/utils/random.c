#include <fudge.h>
#include <abi.h>

static void onmain(unsigned int source, void *mdata, unsigned int msize)
{

    struct event_clockinfo clockinfo;
    struct mtwist_state state;
    unsigned int value;

    channel_send(option_getdecimal("clock-service"), EVENT_INFO);
    channel_wait_buffer(EVENT_CLOCKINFO, sizeof (struct event_clockinfo), &clockinfo);
    mtwist_seed1(&state, time_unixtime(clockinfo.year, clockinfo.month, clockinfo.day, clockinfo.hours, clockinfo.minutes, clockinfo.seconds));

    value = mtwist_rand(&state);

    channel_send_fmt1(source, EVENT_DATA, "%u\n", &value);

}

void init(void)
{

    option_add("clock-service", "220");
    channel_bind(EVENT_MAIN, onmain);

}

