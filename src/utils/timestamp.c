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

static unsigned int gettimestamp(unsigned int year, unsigned int month, unsigned int day, unsigned int hours, unsigned int minutes, unsigned int seconds)
{

    unsigned int syear = year - 1970;
    unsigned int dyear = ((((365 * syear) + (syear / 4)) - (syear / 100)) + (syear / 400));
    unsigned int dmonth = isleapyear(syear) ? dotm366[month - 1] : dotm365[month - 1];

    return ((dyear + dmonth + day) * 86400) + ((hours * 3600) + (minutes * 60) + seconds);

}

static void onmain(unsigned int source, void *mdata, unsigned int msize)
{

    struct ctrl_clocksettings settings;
    struct message message;

    if (!file_walk2(FILE_L0, option_getstring("clock")))
        channel_error("Could not find clock device ctrl");

    file_readall(FILE_L0, &settings, sizeof (struct ctrl_clocksettings));
    message_init(&message, EVENT_DATA);
    message_putvalue(&message, gettimestamp(settings.year, settings.month, settings.day, settings.hours, settings.minutes, settings.seconds), 10, 0);
    message_putstring(&message, "\n");
    channel_sendmessage(&message);
    channel_close();

}

void init(void)
{

    option_add("clock", "system:clock/if:0/ctrl");
    channel_bind(EVENT_MAIN, onmain);

}

