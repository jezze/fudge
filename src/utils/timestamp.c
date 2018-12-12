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

static void replytimestamp(union event_message *imessage, union event_message *omessage, struct ctrl_clocksettings *settings)
{

    char num[FUDGE_NSIZE];

    event_reply(omessage, imessage, EVENT_DATA);
    event_append(omessage, ascii_wvalue(num, FUDGE_NSIZE, gettimestamp(settings), 10), num);
    event_append(omessage, 1, "\n");
    event_place(omessage->header.target, omessage);

}

static unsigned int onempty(union event_message *imessage, union event_message *omessage)
{

    struct ctrl_clocksettings settings;

    if (!file_walk2(FILE_L0, "/system/clock/if:0/ctrl"))
        return 0;

    file_open(FILE_L0);
    file_readall(FILE_L0, &settings, sizeof (struct ctrl_clocksettings));
    file_close(FILE_L0);
    replytimestamp(imessage, omessage, &settings);

    return 0;

}

static unsigned int onfile(union event_message *imessage, union event_message *omessage)
{

    struct event_file *file = event_getdata(imessage);
    struct ctrl_clocksettings settings;

    file_open(file->descriptor);
    file_readall(file->descriptor, &settings, sizeof (struct ctrl_clocksettings));
    file_close(file->descriptor);
    replytimestamp(imessage, omessage, &settings);

    return 0;

}

static unsigned int onstop(union event_message *imessage, union event_message *omessage)
{

    return 1;

}

static unsigned int onkill(union event_message *imessage, union event_message *omessage)
{

    return 1;

}

void main(void)
{

    unsigned int status = 0;
    union event_message imessage;
    union event_message omessage;

    while (!status)
    {

        switch (event_pick(&imessage))
        {

        case EVENT_EMPTY:
            status = onempty(&imessage, &omessage);

            break;

        case EVENT_FILE:
            status = onfile(&imessage, &omessage);

            break;

        case EVENT_STOP:
            status = onstop(&imessage, &omessage);

            break;

        case EVENT_KILL:
            status = onkill(&imessage, &omessage);

            break;

        }

    }

}

