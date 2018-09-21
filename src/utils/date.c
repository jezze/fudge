#include <abi.h>
#include <fudge.h>

static unsigned int quit;

static void date(struct event_header *header, unsigned int descriptor)
{

    struct ctrl_clocksettings settings;
    char *datetime = "0000-00-00 00:00:00\n";
    char message[FUDGE_BSIZE];

    file_open(descriptor);
    file_readall(descriptor, &settings, sizeof (struct ctrl_clocksettings));
    file_close(descriptor);
    ascii_wzerovalue(datetime, 20, settings.year, 10, 4, 0);
    ascii_wzerovalue(datetime, 20, settings.month, 10, 2, 5);
    ascii_wzerovalue(datetime, 20, settings.day, 10, 2, 8);
    ascii_wzerovalue(datetime, 20, settings.hours, 10, 2, 11);
    ascii_wzerovalue(datetime, 20, settings.minutes, 10, 2, 14);
    ascii_wzerovalue(datetime, 20, settings.seconds, 10, 2, 17);
    event_addresponse(message, header, EVENT_DATA);
    event_adddata(message, 20, datetime);
    event_send(message);

}

static void onkill(struct event_header *header)
{

    char message[FUDGE_BSIZE];

    event_addresponse(message, header, EVENT_CHILD);
    event_send(message);

    quit = 1;

}

static void onfile(struct event_header *header)
{

    struct event_file *file = event_getdata(header);

    if (file->descriptor)
    {

        date(header, file->descriptor);

    }

    else
    {

        if (!file_walk(FILE_L0, "/system/clock/if:0/ctrl"))
            return;

        date(header, FILE_L0);

    }

}

void main(void)
{

    event_open();

    while (!quit)
    {

        char data[FUDGE_BSIZE];
        struct event_header *header = event_read(data);

        switch (header->type)
        {

        case EVENT_EXIT:
        case EVENT_KILL:
            onkill(header);

            break;

        case EVENT_FILE:
            onfile(header);

            break;

        }

    }

    event_close();

}

