#include <fudge.h>
#include <abi.h>

static void replydate(struct channel *channel, struct ctrl_clocksettings *settings)
{

    char num[FUDGE_NSIZE];

    channel_reply(channel, EVENT_DATA);
    event_append(&channel->o.header, ascii_wzerovalue(num, FUDGE_NSIZE, settings->year, 10, 4, 0), num);
    event_append(&channel->o.header, 1, "-");
    event_append(&channel->o.header, ascii_wzerovalue(num, FUDGE_NSIZE, settings->month, 10, 2, 0), num);
    event_append(&channel->o.header, 1, "-");
    event_append(&channel->o.header, ascii_wzerovalue(num, FUDGE_NSIZE, settings->day, 10, 2, 0), num);
    event_append(&channel->o.header, 1, " ");
    event_append(&channel->o.header, ascii_wzerovalue(num, FUDGE_NSIZE, settings->hours, 10, 2, 0), num);
    event_append(&channel->o.header, 1, ":");
    event_append(&channel->o.header, ascii_wzerovalue(num, FUDGE_NSIZE, settings->minutes, 10, 2, 0), num);
    event_append(&channel->o.header, 1, ":");
    event_append(&channel->o.header, ascii_wzerovalue(num, FUDGE_NSIZE, settings->seconds, 10, 2, 0), num);
    event_append(&channel->o.header, 1, "\n");
    channel_place(channel->o.header.target, &channel->o);

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

    struct event_file *file = mdata;
    struct ctrl_clocksettings settings;

    file_open(file->descriptor);
    file_readall(file->descriptor, &settings, sizeof (struct ctrl_clocksettings));
    file_close(file->descriptor);
    replydate(channel, &settings);

}

void main(void)
{

    struct channel channel;

    channel_init(&channel);
    channel_setsignal(&channel, EVENT_EMPTY, onempty);
    channel_setsignal(&channel, EVENT_FILE, onfile);
    channel_listen(&channel);

}

