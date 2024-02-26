#include <fudge.h>
#include <abi.h>

static void seed(struct mtwist_state *state)
{

    struct ctrl_clocksettings settings;

    if (!call_walk_absolute(FILE_L0, option_getstring("clock")))
        PANIC();

    if (!call_walk_relative(FILE_L1, FILE_L0, "ctrl"))
        PANIC();

    call_read_all(FILE_L1, &settings, sizeof (struct ctrl_clocksettings), 0);
    mtwist_seed1(state, time_unixtime(settings.year, settings.month, settings.day, settings.hours, settings.minutes, settings.seconds));

}

static void onmain(unsigned int source, void *mdata, unsigned int msize)
{

    struct mtwist_state state;
    unsigned int value;

    seed(&state);

    value = mtwist_rand(&state);

    channel_send_fmt1(CHANNEL_DEFAULT, EVENT_DATA, "%u\n", &value);

}

void init(void)
{

    option_add("clock", "system:clock/if.0");
    channel_bind(EVENT_MAIN, onmain);

}

