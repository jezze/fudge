#include <fudge.h>
#include <abi.h>

static void onmain(unsigned int source, void *mdata, unsigned int msize)
{

    struct ctrl_clocksettings settings;

    channel_send(option_getdecimal("clock-service"), EVENT_INFO);
    channel_wait_buffer(EVENT_CLOCKINFO, sizeof (struct ctrl_clocksettings), &settings);
    channel_send_fmt6(source, EVENT_DATA, "%4h-%2c-%2c %2c:%2c:%2c\n", &settings.year, &settings.month, &settings.day, &settings.hours, &settings.minutes, &settings.seconds);

}

void init(void)
{

    option_add("clock-service", "220");
    channel_bind(EVENT_MAIN, onmain);

}

