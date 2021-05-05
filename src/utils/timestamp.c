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

    offset = message_putvalue(&data, gettimestamp(settings), 10, 0, offset);
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
        file_walk2(FILE_G0, value);

}

void init(struct channel *channel)
{

    file_walk2(FILE_G0, "system:clock/if:0");
    channel_setcallback(channel, EVENT_MAIN, onmain);
    channel_setcallback(channel, EVENT_OPTION, onoption);

}

