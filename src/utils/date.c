#include <fudge.h>
#include <abi.h>

static void onmain(unsigned int source, void *mdata, unsigned int msize)
{

    if (file_walk2(FILE_L0, option_getstring("clock")))
    {

        struct ctrl_clocksettings settings;
        struct message message;

        file_readall(FILE_L0, &settings, sizeof (struct ctrl_clocksettings));
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

    }

    else
    {

        channel_error("Clock ctrl file not found");

    }

    channel_close();

}

void init(void)
{

    option_add("clock", "system:clock/if:0/ctrl");
    channel_bind(EVENT_MAIN, onmain);

}

