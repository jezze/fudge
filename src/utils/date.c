#include <fudge.h>
#include <abi.h>

static void onmain(unsigned int source, void *mdata, unsigned int msize)
{

    struct ctrl_clocksettings settings;
    struct message message;

    if (!file_walk2(FILE_L0, option_getstring("clock")))
        channel_error("Could not find clock device");

    if (!file_walk(FILE_L1, FILE_L0, "ctrl"))
        channel_error("Could not find clock device ctrl");

    file_readall(FILE_L1, &settings, sizeof (struct ctrl_clocksettings));
    message_init(&message, EVENT_DATA);
    message_putvalue(&message, settings.year, 10, 4);
    message_putstring(&message, "-");
    message_putvalue(&message, settings.month, 10, 2);
    message_putstring(&message, "-");
    message_putvalue(&message, settings.day, 10, 2);
    message_putstring(&message, " ");
    message_putvalue(&message, settings.hours, 10, 2);
    message_putstring(&message, ":");
    message_putvalue(&message, settings.minutes, 10, 2);
    message_putstring(&message, ":");
    message_putvalue(&message, settings.seconds, 10, 2);
    message_putstring(&message, "\n");
    channel_sendmessage(&message);
    channel_close();

}

void init(void)
{

    option_add("clock", "system:clock/if:0");
    channel_bind(EVENT_MAIN, onmain);

}

