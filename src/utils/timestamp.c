#include <abi.h>
#include <fudge.h>

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

static unsigned int gettimestamp(struct ctrl_clocksettings *settings)
{

    unsigned int year = settings->year - 1970;
    unsigned int dyear = ((((365 * year) + (year / 4)) - (year / 100)) + (year / 400));
    unsigned int dmonth = isleapyear(year) ? dotm366[settings->month - 1] : dotm365[settings->month - 1];

    return ((dyear + dmonth + settings->day) * 86400) + ((settings->hours * 3600) + (settings->minutes * 60) + settings->seconds);

}

static void ondatafile(struct event_header *iheader, struct event_header *oheader)
{

    struct event_datafile *datafile = event_getdata(iheader);
    struct ctrl_clocksettings settings;
    char num[FUDGE_NSIZE];

    if (!datafile->descriptor)
    {

        if (!file_walk(FILE_L0, "/system/clock/if:0/ctrl"))
            return;

        datafile->descriptor = FILE_L0;

    }

    file_open(datafile->descriptor);
    file_readall(datafile->descriptor, &settings, sizeof (struct ctrl_clocksettings));
    file_close(datafile->descriptor);
    event_reply(oheader, iheader, EVENT_DATAPIPE);
    event_adddatapipe(oheader, datafile->session);
    event_appenddata(oheader, ascii_wvalue(num, FUDGE_NSIZE, gettimestamp(&settings), 10), num);
    event_appenddata(oheader, 1, "\n");
    event_send(oheader);

}

static void ondatastop(struct event_header *iheader, struct event_header *oheader)
{

    struct event_datastop *datastop = event_getdata(iheader);

    event_reply(oheader, iheader, EVENT_DATASTOP);
    event_adddatastop(oheader, datastop->session);
    event_send(oheader);

    quit = 1;

}

static void onkill(struct event_header *iheader, struct event_header *oheader)
{

    quit = 1;

}

void main(void)
{

    event_open();

    while (!quit)
    {

        char ibuffer[FUDGE_BSIZE];
        char obuffer[FUDGE_BSIZE];
        struct event_header *iheader = event_read(ibuffer);
        struct event_header *oheader = (struct event_header *)obuffer;

        switch (iheader->type)
        {

        case EVENT_DATAFILE:
            ondatafile(iheader, oheader);

            break;

        case EVENT_DATASTOP:
            ondatastop(iheader, oheader);

            break;

        case EVENT_KILL:
            onkill(iheader, oheader);

            break;

        }

    }

    event_close();

}

