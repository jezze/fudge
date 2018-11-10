#include <fudge.h>
#include <abi.h>

static unsigned int onfile(union event_message *imessage, union event_message *omessage)
{

    struct event_file *file = event_getdata(imessage);
    struct ctrl_clocksettings settings;
    char num[FUDGE_NSIZE];

    if (!file->descriptor)
    {

        if (!file_walk2(FILE_L0, "/system/clock/if:0/ctrl"))
            return 0;

        file->descriptor = FILE_L0;

    }

    file_open(file->descriptor);
    file_readall(file->descriptor, &settings, sizeof (struct ctrl_clocksettings));
    file_close(file->descriptor);
    event_reply(omessage, imessage, EVENT_DATA);
    event_append(omessage, ascii_wzerovalue(num, FUDGE_NSIZE, settings.year, 10, 4, 0), num);
    event_append(omessage, 1, "-");
    event_append(omessage, ascii_wzerovalue(num, FUDGE_NSIZE, settings.month, 10, 2, 0), num);
    event_append(omessage, 1, "-");
    event_append(omessage, ascii_wzerovalue(num, FUDGE_NSIZE, settings.day, 10, 2, 0), num);
    event_append(omessage, 1, " ");
    event_append(omessage, ascii_wzerovalue(num, FUDGE_NSIZE, settings.hours, 10, 2, 0), num);
    event_append(omessage, 1, ":");
    event_append(omessage, ascii_wzerovalue(num, FUDGE_NSIZE, settings.minutes, 10, 2, 0), num);
    event_append(omessage, 1, ":");
    event_append(omessage, ascii_wzerovalue(num, FUDGE_NSIZE, settings.seconds, 10, 2, 0), num);
    event_append(omessage, 1, "\n");
    event_send(omessage);

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

    event_open();

    while (!status)
    {

        switch (event_read(&imessage))
        {

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

    event_close();

}

