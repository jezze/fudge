#include <fudge.h>
#include <abi.h>

static unsigned int onfile(struct event_header *iheader, struct event_header *oheader)
{

    struct event_file *file = event_getdata(iheader);
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
    event_replydata(oheader, iheader);
    event_appenddata(oheader, ascii_wzerovalue(num, FUDGE_NSIZE, settings.year, 10, 4, 0), num);
    event_appenddata(oheader, 1, "-");
    event_appenddata(oheader, ascii_wzerovalue(num, FUDGE_NSIZE, settings.month, 10, 2, 0), num);
    event_appenddata(oheader, 1, "-");
    event_appenddata(oheader, ascii_wzerovalue(num, FUDGE_NSIZE, settings.day, 10, 2, 0), num);
    event_appenddata(oheader, 1, " ");
    event_appenddata(oheader, ascii_wzerovalue(num, FUDGE_NSIZE, settings.hours, 10, 2, 0), num);
    event_appenddata(oheader, 1, ":");
    event_appenddata(oheader, ascii_wzerovalue(num, FUDGE_NSIZE, settings.minutes, 10, 2, 0), num);
    event_appenddata(oheader, 1, ":");
    event_appenddata(oheader, ascii_wzerovalue(num, FUDGE_NSIZE, settings.seconds, 10, 2, 0), num);
    event_appenddata(oheader, 1, "\n");
    event_send(oheader);

    return 0;

}

static unsigned int onstop(struct event_header *iheader, struct event_header *oheader)
{

    return 1;

}

static unsigned int onkill(struct event_header *iheader, struct event_header *oheader)
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

        event_read(&imessage);

        switch (imessage.header.type)
        {

        case EVENT_FILE:
            status = onfile(&imessage.header, &omessage.header);

            break;

        case EVENT_STOP:
            status = onstop(&imessage.header, &omessage.header);

            break;

        case EVENT_KILL:
            status = onkill(&imessage.header, &omessage.header);

            break;

        }

    }

    event_close();

}

