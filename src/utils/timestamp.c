#include <fudge.h>
#include <abi.h>

static void onmain(unsigned int source, void *mdata, unsigned int msize)
{

    unsigned int clock = channel_lookup(option_getstring("clock-service"));

    if (clock)
    {

        struct event_clockinfo clockinfo;
        unsigned int timestamp;

        channel_send(0, clock, EVENT_INFO, 0, 0);
        channel_wait(0, clock, EVENT_CLOCKINFO, sizeof (struct event_clockinfo), &clockinfo);

        timestamp = time_unixtime(clockinfo.year, clockinfo.month, clockinfo.day, clockinfo.hours, clockinfo.minutes, clockinfo.seconds);

        channel_send_fmt1(0, source, EVENT_DATA, "%u\n", &timestamp);

    }

}

void init(void)
{

    option_add("clock-service", "clock");
    channel_bind(EVENT_MAIN, onmain);

    while (channel_process(0));

}

