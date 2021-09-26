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

static void print(unsigned int source, struct ctrl_clocksettings *settings)
{

    struct message message;

    message_init(&message, EVENT_DATA);
    message_putvalue2(&message, gettimestamp(settings), 10, 0);
    message_putstring2(&message, "\n");
    channel_replymsg(&message);

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

