#include <fudge.h>
#include <abi.h>

static void list(struct event_header *iheader, struct event_header *oheader, unsigned int descriptor)
{

    struct record record;

    file_open(descriptor);
    event_replydata(oheader, iheader);

    while (file_readall(descriptor, &record, sizeof (struct record)))
    {

        char num[FUDGE_NSIZE];

        if (event_avail(oheader) < record.length + 3 + 16)
        {

            event_send(oheader);
            event_replydata(oheader, iheader);

        }

        event_appenddata(oheader, ascii_wzerovalue(num, FUDGE_NSIZE, record.id, 16, 8, 0), num);
        event_appenddata(oheader, 1, " ");
        event_appenddata(oheader, ascii_wzerovalue(num, FUDGE_NSIZE, record.size, 16, 8, 0), num);
        event_appenddata(oheader, 1, " ");
        event_appenddata(oheader, record.length, record.name);
        event_appenddata(oheader, 1, "\n");

        if (!file_step(descriptor))
            break;

    }

    event_send(oheader);
    file_close(descriptor);

}

static unsigned int onfile(struct event_header *iheader, struct event_header *oheader)
{

    struct event_file *file = event_getdata(iheader);

    if (file->descriptor)
        list(iheader, oheader, file->descriptor);
    else
        list(iheader, oheader, FILE_PW);

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

