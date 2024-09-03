#include <fudge.h>
#include <abi.h>

struct ctrl_clocksettings settings;

static void onclockinfo(unsigned int source, void *mdata, unsigned int msize)
{

    buffer_copy(&settings, mdata, msize);

}

static void onmain(unsigned int source, void *mdata, unsigned int msize)
{

    unsigned int timestamp;

    channel_send(option_getdecimal("clock-service"), EVENT_INFO);
    channel_wait(EVENT_CLOCKINFO);

    timestamp = time_unixtime(settings.year, settings.month, settings.day, settings.hours, settings.minutes, settings.seconds);

    channel_send_fmt1(source, EVENT_DATA, "%u\n", &timestamp);

}

void init(void)
{

    option_add("clock-service", "220");
    channel_bind(EVENT_CLOCKINFO, onclockinfo);
    channel_bind(EVENT_MAIN, onmain);

}

