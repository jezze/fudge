#include <fudge.h>
#include <abi.h>

static void onmain(unsigned int source, void *mdata, unsigned int msize)
{

    struct event_clockinfo clockinfo;
    struct mtwist_state state;
    unsigned int value;

    lookup2("clock-service", "clock", 0, 0);
    channel_send(0, option_getdecimal("clock-service"), EVENT_INFO);
    channel_wait_buffer(0, option_getdecimal("clock-service"), EVENT_CLOCKINFO, sizeof (struct event_clockinfo), &clockinfo);
    mtwist_seed1(&state, time_unixtime(clockinfo.year, clockinfo.month, clockinfo.day, clockinfo.hours, clockinfo.minutes, clockinfo.seconds));

    value = mtwist_rand(&state);

    channel_send_fmt1(0, source, EVENT_DATA, "%u\n", &value);

}

void init(void)
{

    option_add("clock-service", "");
    channel_bind(EVENT_MAIN, onmain);

}

