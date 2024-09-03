#include <fudge.h>
#include <abi.h>

struct ctrl_clocksettings settings;

static void onclockinfo(unsigned int source, void *mdata, unsigned int msize)
{

    buffer_copy(&settings, mdata, msize);

}

static void onmain(unsigned int source, void *mdata, unsigned int msize)
{

    channel_send(option_getdecimal("clock-service"), EVENT_CTRL);
    channel_wait(EVENT_CLOCKINFO);
    channel_send_fmt6(source, EVENT_DATA, "%4h-%2c-%2c %2c:%2c:%2c\n", &settings.year, &settings.month, &settings.day, &settings.hours, &settings.minutes, &settings.seconds);

}

void init(void)
{

    option_add("clock-service", "220");
    channel_bind(EVENT_CLOCKINFO, onclockinfo);
    channel_bind(EVENT_MAIN, onmain);

}

