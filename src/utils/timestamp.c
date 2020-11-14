#include <fudge.h>
#include <abi.h>

static unsigned int dotm365[13] = {0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365};
static unsigned int dotm366[13] = {0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335, 365};

static unsigned int isleapyear(unsigned int year)
{

    if ((year % 4) != 0)
        return 0;

    if ((year % 100) == 0)
        return ((year % 400) == 0);

    return 1;

}

static unsigned int gettimestamp(struct ctrl_clocksettings *settings)
{

    unsigned int year = settings->year - 1970;
    unsigned int dyear = ((((365 * year) + (year / 4)) - (year / 100)) + (year / 400));
    unsigned int dmonth = isleapyear(year) ? dotm366[settings->month - 1] : dotm365[settings->month - 1];

    return ((dyear + dmonth + settings->day) * 86400) + ((settings->hours * 3600) + (settings->minutes * 60) + settings->seconds);

}

static void print(struct channel *channel, unsigned int source, struct ctrl_clocksettings *settings)
{

    struct message_data data;
    unsigned int offset = 0;

    offset = message_appendvalue(&data, gettimestamp(settings), 10, 0, offset);
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

