#include <abi.h>
#include <fudge.h>
#include <event/base.h>

static unsigned int dotm365[13] = {0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365};
static unsigned int dotm366[13] = {0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335, 365};
static unsigned int quit;

static unsigned int isleapyear(unsigned int year)
{

    if ((year % 4) != 0)
        return 0;

    if ((year % 100) == 0)
        return ((year % 400) == 0);

    return 1;

}

static void timestamp(struct event_header *header, struct ctrl_clocksettings *settings)
{

    unsigned int year = settings->year - 1970;
    unsigned int dyear = ((((365 * year) + (year / 4)) - (year / 100)) + (year / 400));
    unsigned int dmonth = isleapyear(year) ? dotm366[settings->month - 1] : dotm365[settings->month - 1];
    unsigned int timestamp = ((dyear + dmonth + settings->day) * 86400) + ((settings->hours * 3600) + (settings->minutes * 60) + settings->seconds);
    unsigned int count = 0;
    char message[FUDGE_BSIZE];
    char num[FUDGE_NSIZE];

    count += ascii_wvalue(num, FUDGE_NSIZE, timestamp, 10);
    count += memory_write(num, FUDGE_NSIZE, "\n", 1, count);

    event_addreply(message, header, EVENT_DATA);
    event_adddata(message, count, num);
    event_sendbuffer(FILE_L0, message);

}

static void onkill(struct event_header *header)
{

    event_reply(FILE_L0, header, EVENT_CHILD);

    quit = 1;

}

static void ondata(struct event_header *header)
{

    struct ctrl_clocksettings settings;

    if (!file_walk(FILE_L1, "/system/clock/if:0/ctrl"))
        return;

    file_open(FILE_L1);
    file_readall(FILE_L1, &settings, sizeof (struct ctrl_clocksettings));
    file_close(FILE_L1);
    timestamp(header, &settings);

}

void main(void)
{

    /* Once event system is inside kernel, there is no need to use the filesystem to send events */
    file_walk(FILE_L0, "/system/event");
    file_open(FILE_L0);

    while (!quit)
    {

        char data[FUDGE_BSIZE];
        struct event_header *header = event_read(FILE_L0, data);

        switch (header->type)
        {

        case EVENT_EXIT:
        case EVENT_KILL:
            onkill(header);

            break;

        case EVENT_DATA:
            ondata(header);

            break;

        }

    }

    file_close(FILE_L0);

}

