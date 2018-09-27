#include <abi.h>
#include <fudge.h>

static unsigned int quit;

static void onkill(struct event_header *header, void *message)
{

    event_addresponse(message, header, EVENT_EXIT);
    event_send(message);

    quit = 1;

}

static void onfile(struct event_header *header, void *message)
{

    struct event_file *file = event_getdata(header);
    struct ctrl_clocksettings settings;
    char *datetime = "0000-00-00 00:00:00\n";

    if (!file->descriptor)
    {

        if (!file_walk(FILE_L0, "/system/clock/if:0/ctrl"))
            return;

        file->descriptor = FILE_L0;

    }

    file_open(file->descriptor);
    file_readall(file->descriptor, &settings, sizeof (struct ctrl_clocksettings));
    file_close(file->descriptor);
    ascii_wzerovalue(datetime, 20, settings.year, 10, 4, 0);
    ascii_wzerovalue(datetime, 20, settings.month, 10, 2, 5);
    ascii_wzerovalue(datetime, 20, settings.day, 10, 2, 8);
    ascii_wzerovalue(datetime, 20, settings.hours, 10, 2, 11);
    ascii_wzerovalue(datetime, 20, settings.minutes, 10, 2, 14);
    ascii_wzerovalue(datetime, 20, settings.seconds, 10, 2, 17);
    event_addresponse(message, header, EVENT_DATA);
    event_adddata(message, file->session);
    event_appenddata(message, 20, datetime);
    event_send(message);

}

void main(void)
{

    event_open();

    while (!quit)
    {

        char data[FUDGE_BSIZE];
        char message[FUDGE_BSIZE];
        struct event_header *header = event_read(data);

        switch (header->type)
        {

        case EVENT_EXIT:
        case EVENT_KILL:
            onkill(header, message);

            break;

        case EVENT_FILE:
            onfile(header, message);

            break;

        }

    }

    event_close();

}

