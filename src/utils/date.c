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

    channel_place(channel, EVENT_DATA, offset, &data);

}

static void onmain(struct channel *channel, unsigned int source, void *mdata, unsigned int msize)
{

    if (file_walk2(FILE_L0, "/system/clock/if:0/ctrl"))
    {

        struct ctrl_clocksettings settings;

        file_open(FILE_L0);
        file_readall(FILE_L0, &settings, sizeof (struct ctrl_clocksettings));
        file_close(FILE_L0);
        print(channel, source, &settings);

    }

    channel_close(channel);

}

static void onmain2(struct channel *channel, unsigned int source, void *mdata, unsigned int msize)
{

    channel_close(channel);

}

static void onfile(struct channel *channel, unsigned int source, void *mdata, unsigned int msize)
{

    if (file_walk2(FILE_L0, mdata))
    {

        struct ctrl_clocksettings settings;

        file_open(FILE_L0);
        file_readall(FILE_L0, &settings, sizeof (struct ctrl_clocksettings));
        file_close(FILE_L0);
        print(channel, source, &settings);

    }

    channel_setcallback(channel, EVENT_MAIN, onmain2);

}

void init(struct channel *channel)
{

    channel_setcallback(channel, EVENT_MAIN, onmain);
    channel_setcallback(channel, EVENT_FILE, onfile);

}

