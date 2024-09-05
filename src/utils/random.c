#include <fudge.h>
#include <abi.h>

static void onmain(unsigned int source, void *mdata, unsigned int msize)
{

    struct ctrl_clocksettings settings;
    struct mtwist_state state;
    unsigned int value;

    channel_send(option_getdecimal("clock-service"), EVENT_INFO);
    channel_wait_buffer(EVENT_CLOCKINFO, sizeof (struct ctrl_clocksettings), &settings);
    mtwist_seed1(&state, time_unixtime(settings.year, settings.month, settings.day, settings.hours, settings.minutes, settings.seconds));

    value = mtwist_rand(&state);

    channel_send_fmt1(source, EVENT_DATA, "%u\n", &value);

}

void init(void)
{

    option_add("clock-service", "220");
    channel_bind(EVENT_MAIN, onmain);

}

