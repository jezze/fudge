#include <fudge.h>
#include <abi.h>

static void onmain(unsigned int source, void *mdata, unsigned int msize)
{

    struct ctrl_clocksettings settings;
    struct message message;
    struct mtwist_state state;

    if (!file_walk2(FILE_L0, option_getstring("clock")))
        channel_error("Could not find clock device ctrl");

    file_readall(FILE_L0, &settings, sizeof (struct ctrl_clocksettings));
    mtwist_seed1(&state, time_unixtime(settings.year, settings.month, settings.day, settings.hours, settings.minutes, settings.seconds));
    message_init(&message, EVENT_DATA);
    message_putvalue(&message, mtwist_rand(&state), 10, 0);
    message_putstring(&message, "\n");
    channel_sendmessage(&message);
    channel_close();

}

void init(void)
{

    option_add("clock", "system:clock/if:0/ctrl");
    channel_bind(EVENT_MAIN, onmain);

}

