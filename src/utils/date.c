#include <fudge.h>
#include <abi.h>

static void print(struct channel *channel, unsigned int source, struct ctrl_clocksettings *settings)
{

    struct message_data data;
    unsigned int offset = 0;

    offset = message_putvalue(&data, settings->year, 10, 4, offset);
    offset = message_putstring(&data, "-", offset);
    offset = message_putvalue(&data, settings->month, 10, 2, offset);
    offset = message_putstring(&data, "-", offset);
    offset = message_putvalue(&data, settings->day, 10, 2, offset);
    offset = message_putstring(&data, " ", offset);
    offset = message_putvalue(&data, settings->hours, 10, 2, offset);
    offset = message_putstring(&data, ":", offset);
    offset = message_putvalue(&data, settings->minutes, 10, 2, offset);
    offset = message_putstring(&data, ":", offset);
    offset = message_putvalue(&data, settings->seconds, 10, 2, offset);
    offset = message_putstring(&data, "\n", offset);

    channel_reply(channel, EVENT_DATA, offset, &data);

}

static void onmain(struct channel *channel, unsigned int source, void *mdata, unsigned int msize)
{

    if (file_walk(FILE_L0, FILE_G0, "ctrl"))
    {

        struct ctrl_clocksettings settings;

        file_open(FILE_L0);
        file_readall(FILE_L0, &settings, sizeof (struct ctrl_clocksettings));
        file_close(FILE_L0);
        print(channel, source, &settings);

    }

    channel_close(channel);

}

static void onoption(struct channel *channel, unsigned int source, void *mdata, unsigned int msize)
{

    char *key = mdata;
    char *value = key + ascii_lengthz(key);

    if (ascii_match(key, "clock"))
    {

        file_walk2(FILE_G0, value);

    }

}

void init(struct channel *channel)
{

    file_walk2(FILE_G0, "system:clock/if:0");
    channel_setcallback(channel, EVENT_MAIN, onmain);
    channel_setcallback(channel, EVENT_OPTION, onoption);

}

