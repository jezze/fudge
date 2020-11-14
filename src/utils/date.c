#include <fudge.h>
#include <abi.h>

static void print(struct channel *channel, unsigned int source, struct ctrl_clocksettings *settings)
{

    struct message_data data;
    unsigned int offset = 0;

    offset = message_appendvalue(&data, settings->year, 10, 4, offset);
    offset = message_appendstring(&data, "-", offset);
    offset = message_appendvalue(&data, settings->month, 10, 2, offset);
    offset = message_appendstring(&data, "-", offset);
    offset = message_appendvalue(&data, settings->day, 10, 2, offset);
    offset = message_appendstring(&data, " ", offset);
    offset = message_appendvalue(&data, settings->hours, 10, 2, offset);
    offset = message_appendstring(&data, ":", offset);
    offset = message_appendvalue(&data, settings->minutes, 10, 2, offset);
    offset = message_appendstring(&data, ":", offset);
    offset = message_appendvalue(&data, settings->seconds, 10, 2, offset);
    offset = message_appendstring(&data, "\n", offset);

    channel_place(channel, source, EVENT_DATA, offset, &data);

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

    channel_setsignal(channel, EVENT_MAIN, onmain2);

}

void main(void)
{

    struct channel channel;

    channel_init(&channel);
    channel_setsignal(&channel, EVENT_MAIN, onmain);
    channel_setsignal(&channel, EVENT_FILE, onfile);
    channel_listen(&channel, 0, 0);

}

