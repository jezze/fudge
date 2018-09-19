#include <abi.h>
#include <fudge.h>
#include <event/base.h>

static unsigned int quit;

static void date(struct event_header *header, struct ctrl_clocksettings *settings)
{

    char message[FUDGE_BSIZE];
    char *datetime = "0000-00-00 00:00:00\n";

    ascii_wzerovalue(datetime, 20, settings->year, 10, 4, 0);
    ascii_wzerovalue(datetime, 20, settings->month, 10, 2, 5);
    ascii_wzerovalue(datetime, 20, settings->day, 10, 2, 8);
    ascii_wzerovalue(datetime, 20, settings->hours, 10, 2, 11);
    ascii_wzerovalue(datetime, 20, settings->minutes, 10, 2, 14);
    ascii_wzerovalue(datetime, 20, settings->seconds, 10, 2, 17);
    event_addreply(message, header, EVENT_DATA);
    event_adddata(message, 20, datetime);
    event_sendbuffer(FILE_L0, message);

}

static void onkill(struct event_header *header)
{

    char message[FUDGE_BSIZE];

    event_addreply(message, header, EVENT_CHILD);
    event_sendbuffer(FILE_L0, message);

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
    date(header, &settings);

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

