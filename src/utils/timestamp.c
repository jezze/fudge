#include <fudge.h>
#include <abi.h>

static void onmain(unsigned int source, void *mdata, unsigned int msize)
{

    struct event_clockinfo clockinfo;
    unsigned int timestamp;

    option_setdecimal("clock-service", lookup(option_getstring("clock-service")));
    channel_send(0, option_getdecimal("clock-service"), EVENT_INFO);
    channel_wait_buffer(0, option_getdecimal("clock-service"), EVENT_CLOCKINFO, sizeof (struct event_clockinfo), &clockinfo);

    timestamp = time_unixtime(clockinfo.year, clockinfo.month, clockinfo.day, clockinfo.hours, clockinfo.minutes, clockinfo.seconds);

    channel_send_fmt1(0, source, EVENT_DATA, "%u\n", &timestamp);

}

void init(void)
{

    option_add("clock-service", "clock");
    channel_bind(EVENT_MAIN, onmain);

}

