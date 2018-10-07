#include <abi.h>
#include <fudge.h>

static unsigned int quit;

static void ondatafile(struct event_header *iheader, struct event_header *oheader)
{

    struct event_datafile *datafile = event_getdata(iheader);
    struct ctrl_clocksettings settings;
    char *datetime = "0000-00-00 00:00:00\n";

    if (!datafile->descriptor)
    {

        if (!file_walk(FILE_L0, "/system/clock/if:0/ctrl"))
            return;

        datafile->descriptor = FILE_L0;

    }

    file_open(datafile->descriptor);
    file_readall(datafile->descriptor, &settings, sizeof (struct ctrl_clocksettings));
    file_close(datafile->descriptor);
    ascii_wzerovalue(datetime, 20, settings.year, 10, 4, 0);
    ascii_wzerovalue(datetime, 20, settings.month, 10, 2, 5);
    ascii_wzerovalue(datetime, 20, settings.day, 10, 2, 8);
    ascii_wzerovalue(datetime, 20, settings.hours, 10, 2, 11);
    ascii_wzerovalue(datetime, 20, settings.minutes, 10, 2, 14);
    ascii_wzerovalue(datetime, 20, settings.seconds, 10, 2, 17);
    event_reply(oheader, iheader, EVENT_DATAPIPE);
    event_adddatapipe(oheader, datafile->session);
    event_appenddata(oheader, 20, datetime);
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

