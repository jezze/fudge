#include <fudge.h>
#include <abi.h>

static void onmain(unsigned int source, void *mdata, unsigned int msize)
{

    struct ctrl_clocksettings settings;
    unsigned int timestamp;

    if (!file_walk2(FILE_L0, option_getstring("clock")))
        PANIC();

    if (!file_walk(FILE_L1, FILE_L0, "ctrl"))
        PANIC();

    file_readall(FILE_L1, &settings, sizeof (struct ctrl_clocksettings));

    timestamp = time_unixtime(settings.year, settings.month, settings.day, settings.hours, settings.minutes, settings.seconds);

    channel_sendfmt1(CHANNEL_DEFAULT, EVENT_DATA, "%u\n", &timestamp);
    channel_close();

}

void init(void)
{

    option_add("clock", "system:clock/if:0");
    channel_bind(EVENT_MAIN, onmain);

}

