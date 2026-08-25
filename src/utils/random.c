#include <fudge.h>
#include <abi.h>

static void onmain(unsigned int source, void *mdata, unsigned int msize)
{

    unsigned int clock = channel_lookup(option_getstring("clock-service"));

    if (clock)
    {

        struct event_clockinfo clockinfo;
        struct mtwist_state state;
        unsigned int value;

        channel_send(0, clock, EVENT_INFO);
        channel_wait(0, clock, EVENT_CLOCKINFO, sizeof (struct event_clockinfo), &clockinfo);
        mtwist_seed1(&state, time_unixtime(clockinfo.year, clockinfo.month, clockinfo.day, clockinfo.hours, clockinfo.minutes, clockinfo.seconds));

        value = mtwist_rand(&state);

        channel_send_fmt1(0, source, EVENT_DATA, "%u\n", &value);

    }

}

void init(void)
{

    option_add("clock-service", "clock");
    channel_bind(EVENT_MAIN, onmain);

    while (channel_process(0));

}

