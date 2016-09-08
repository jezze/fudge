#include <abi.h>
#include <fudge.h>

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

void main(void)
{

    struct ctrl_clocksettings settings;
    char num[FUDGE_NSIZE];
    unsigned int year;
    unsigned int dyear;
    unsigned int dmonth;
    unsigned int timestamp;

    if (!file_walk(CALL_L0, "/system/clock/if:0/ctrl"))
        return;

    file_open(CALL_L0);
    file_readall(CALL_L0, &settings, sizeof (struct ctrl_clocksettings));
    file_close(CALL_L0);

    year = settings.year - 1970;
    dyear = ((((365 * year) + (year / 4)) - (year / 100)) + (year / 400));
    dmonth = isleapyear(year) ? dotm366[settings.month - 1] : dotm365[settings.month - 1];
    timestamp = ((dyear + dmonth + settings.day) * 86400) + ((settings.hours * 3600) + (settings.minutes * 60) + settings.seconds);

    file_open(CALL_PO);
    file_writeall(CALL_PO, num, ascii_wvalue(num, FUDGE_NSIZE, timestamp, 10, 0));
    file_writeall(CALL_PO, "\n", 1);
    file_close(CALL_PO);

}

