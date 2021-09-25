#include <fudge.h>
#include <abi.h>

static void print(unsigned int source, struct ctrl_clocksettings *settings)
{

    struct message message;
    unsigned int offset = 0;

    offset = message_putvalue(&message, settings->year, 10, 4, offset);
    offset = message_putstring(&message, "-", offset);
    offset = message_putvalue(&message, settings->month, 10, 2, offset);
    offset = message_putstring(&message, "-", offset);
    offset = message_putvalue(&message, settings->day, 10, 2, offset);
    offset = message_putstring(&message, " ", offset);
    offset = message_putvalue(&message, settings->hours, 10, 2, offset);
    offset = message_putstring(&message, ":", offset);
    offset = message_putvalue(&message, settings->minutes, 10, 2, offset);
    offset = message_putstring(&message, ":", offset);
    offset = message_putvalue(&message, settings->seconds, 10, 2, offset);
    offset = message_putstring(&message, "\n", offset);

    channel_reply(EVENT_DATA, offset, message.data.buffer);

}

static void onmain(unsigned int source, void *mdata, unsigned int msize)
{

    if (file_walk(FILE_L0, FILE_G0, "ctrl"))
    {

        struct ctrl_clocksettings settings;

        file_seekreadall(FILE_L0, &settings, sizeof (struct ctrl_clocksettings), 0);
        print(source, &settings);

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
    channel_setcallback(EVENT_MAIN, onmain);
    channel_setcallback(EVENT_OPTION, onoption);

}

