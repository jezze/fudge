#include <fudge.h>
#include <abi.h>

static void replydate(struct channel *channel, struct ctrl_clocksettings *settings)
{

    unsigned int id = channel_reply(channel, EVENT_DATA);

    channel_appendvalue(channel, settings->year, 10, 4);
    channel_appendstring(channel, "-");
    channel_appendvalue(channel, settings->month, 10, 2);
    channel_appendstring(channel, "-");
    channel_appendvalue(channel, settings->day, 10, 2);
    channel_appendstring(channel, " ");
    channel_appendvalue(channel, settings->hours, 10, 2);
    channel_appendstring(channel, ":");
    channel_appendvalue(channel, settings->minutes, 10, 2);
    channel_appendstring(channel, ":");
    channel_appendvalue(channel, settings->seconds, 10, 2);
    channel_appendstring(channel, "\n");
    channel_place(channel, id);

}

static void ondone(struct channel *channel, void *mdata, unsigned int msize)
{

    channel_close(channel);

}

static void onempty(struct channel *channel, void *mdata, unsigned int msize)
{

    struct ctrl_clocksettings settings;

    if (!file_walk2(FILE_L0, "/system/clock/if:0/ctrl"))
        return;

    file_open(FILE_L0);
    file_readall(FILE_L0, &settings, sizeof (struct ctrl_clocksettings));
    file_close(FILE_L0);
    replydate(channel, &settings);

}

static void onfile(struct channel *channel, void *mdata, unsigned int msize)
{

    struct ctrl_clocksettings settings;

    if (!file_walk2(FILE_L0, mdata))
        return;

    file_open(FILE_L0);
    file_readall(FILE_L0, &settings, sizeof (struct ctrl_clocksettings));
    file_close(FILE_L0);
    replydate(channel, &settings);

}

void main(void)
{

    struct channel channel;

    channel_init(&channel);
    channel_setsignal(&channel, EVENT_DONE, ondone);
    channel_setsignal(&channel, EVENT_EMPTY, onempty);
    channel_setsignal(&channel, EVENT_FILE, onfile);
    channel_listen(&channel);

}

