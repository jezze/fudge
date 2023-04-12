#include <fudge.h>
#include <abi.h>

static void seed(struct mtwist_state *state)
{

    struct ctrl_clocksettings settings;

    if (!file_walk2(FILE_L0, option_getstring("clock")))
        channel_panic();

    if (!file_walk(FILE_L1, FILE_L0, "ctrl"))
        channel_panic();

    file_readall(FILE_L1, &settings, sizeof (struct ctrl_clocksettings));
    mtwist_seed1(state, time_unixtime(settings.year, settings.month, settings.day, settings.hours, settings.minutes, settings.seconds));

}

static void onmain(unsigned int source, void *mdata, unsigned int msize)
{

    struct mtwist_state state;
    unsigned int value;

    seed(&state);

    value = mtwist_rand(&state);

    channel_sendfmt1(CHANNEL_DEFAULT, EVENT_DATA, "%u\n", &value);
    channel_close();

}

void init(void)
{

    option_add("clock", "system:clock/if:0");
    channel_bind(EVENT_MAIN, onmain);

}

