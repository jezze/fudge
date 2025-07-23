#include <fudge.h>
#include <abi.h>

static void onmain(unsigned int source, void *mdata, unsigned int msize)
{

    struct event_clockinfo clockinfo;

    option_setdecimal("clock-service", lookup(option_getstring("clock-service")));
    channel_send(0, option_getdecimal("clock-service"), EVENT_INFO);
    channel_wait_buffer(0, option_getdecimal("clock-service"), EVENT_CLOCKINFO, sizeof (struct event_clockinfo), &clockinfo);
    channel_send_fmt6(0, source, EVENT_DATA, "%4h-%2c-%2c %2c:%2c:%2c\n", &clockinfo.year, &clockinfo.month, &clockinfo.day, &clockinfo.hours, &clockinfo.minutes, &clockinfo.seconds);

}

void init(void)
{

    option_add("clock-service", "clock");
    channel_bind(EVENT_MAIN, onmain);

}

