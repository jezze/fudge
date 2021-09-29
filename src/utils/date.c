#include <fudge.h>
#include <abi.h>

static void onmain(unsigned int source, void *mdata, unsigned int msize)
{

    if (file_walk(FILE_L0, FILE_G0, "ctrl"))
    {

        struct ctrl_clocksettings settings;
        struct message message;

        file_seekreadall(FILE_L0, &settings, sizeof (struct ctrl_clocksettings), 0);
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
        channel_replymessage(&message);

    }

    channel_close();

}

static void onoption(unsigned int source, void *mdata, unsigned int msize)
{

    char *key = mdata;
    char *value = key + ascii_lengthz(key);

    if (ascii_match(key, "clock"))
        file_walk2(FILE_G0, value);

}

void init(void)
{

    file_walk2(FILE_G0, "system:clock/if:0");
    channel_bind(EVENT_MAIN, onmain);
    channel_bind(EVENT_OPTION, onoption);

}

