#include <fudge.h>
#include <abi.h>

static void replydate(struct event_channel *channel, struct ctrl_clocksettings *settings)
{

    char num[FUDGE_NSIZE];

    event_reply(channel, EVENT_DATA);
    event_append(&channel->o, ascii_wzerovalue(num, FUDGE_NSIZE, settings->year, 10, 4, 0), num);
    event_append(&channel->o, 1, "-");
    event_append(&channel->o, ascii_wzerovalue(num, FUDGE_NSIZE, settings->month, 10, 2, 0), num);
    event_append(&channel->o, 1, "-");
    event_append(&channel->o, ascii_wzerovalue(num, FUDGE_NSIZE, settings->day, 10, 2, 0), num);
    event_append(&channel->o, 1, " ");
    event_append(&channel->o, ascii_wzerovalue(num, FUDGE_NSIZE, settings->hours, 10, 2, 0), num);
    event_append(&channel->o, 1, ":");
    event_append(&channel->o, ascii_wzerovalue(num, FUDGE_NSIZE, settings->minutes, 10, 2, 0), num);
    event_append(&channel->o, 1, ":");
    event_append(&channel->o, ascii_wzerovalue(num, FUDGE_NSIZE, settings->seconds, 10, 2, 0), num);
    event_append(&channel->o, 1, "\n");
    event_place(channel->o.header.target, &channel->o);

}

static unsigned int onempty(struct event_channel *channel)
{

    struct ctrl_clocksettings settings;

    if (!file_walk2(FILE_L0, "/system/clock/if:0/ctrl"))
        return 0;

    file_open(FILE_L0);
    file_readall(FILE_L0, &settings, sizeof (struct ctrl_clocksettings));
    file_close(FILE_L0);
    replydate(channel, &settings);

    return 0;

}

static unsigned int onfile(struct event_channel *channel)
{

    struct event_file *file = event_getdata(channel);
    struct ctrl_clocksettings settings;

    file_open(file->descriptor);
    file_readall(file->descriptor, &settings, sizeof (struct ctrl_clocksettings));
    file_close(file->descriptor);
    replydate(channel, &settings);

    return 0;

}

void main(void)
{

    struct event_channel channel;

    event_initsignals(&channel);
    event_setsignal(&channel, EVENT_EMPTY, onempty);
    event_setsignal(&channel, EVENT_FILE, onfile);
    event_listen(&channel);

}

