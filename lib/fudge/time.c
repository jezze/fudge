#include "time.h"

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

unsigned int time_unixtime(unsigned int year, unsigned int month, unsigned int day, unsigned int hours, unsigned int minutes, unsigned int seconds)
{

    unsigned int syear = year - 1970;
    unsigned int dyear = ((((365 * syear) + (syear / 4)) - (syear / 100)) + (syear / 400));
    unsigned int dmonth = isleapyear(syear) ? dotm366[month - 1] : dotm365[month - 1];

    return ((dyear + dmonth + day) * 86400) + ((hours * 3600) + (minutes * 60) + seconds);

}

