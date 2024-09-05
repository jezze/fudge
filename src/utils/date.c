#include <fudge.h>
#include <abi.h>

static void onmain(unsigned int source, void *mdata, unsigned int msize)
{

    struct event_clockinfo clockinfo;

    channel_send(option_getdecimal("clock-service"), EVENT_INFO);
    channel_wait_buffer(EVENT_CLOCKINFO, sizeof (struct event_clockinfo), &clockinfo);
    channel_send_fmt6(source, EVENT_DATA, "%4h-%2c-%2c %2c:%2c:%2c\n", &clockinfo.year, &clockinfo.month, &clockinfo.day, &clockinfo.hours, &clockinfo.minutes, &clockinfo.seconds);

}

void init(void)
{

    option_add("clock-service", "220");
    channel_bind(EVENT_MAIN, onmain);

}

