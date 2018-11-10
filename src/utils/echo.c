#include <fudge.h>
#include <abi.h>

static unsigned int ondata(struct event_header *iheader, struct event_header *oheader)
{

    struct event_data *data = event_getdata(iheader);

    event_replydata(oheader, iheader);
    event_appenddata(oheader, data->count, data + 1);
    event_send(oheader);

    return 0;

}

static unsigned int onfile(struct event_header *iheader, struct event_header *oheader)
{

    struct event_file *file = event_getdata(iheader);
    char buffer[FUDGE_BSIZE];
    unsigned int count;

    if (!file->descriptor)
        return 0;

    file_open(file->descriptor);

    while ((count = file_read(file->descriptor, buffer, FUDGE_BSIZE - sizeof (struct event_header) - sizeof (struct event_data))))
    {

        event_replydata(oheader, iheader);
        event_appenddata(oheader, count, buffer);
        event_send(oheader);

    }

    file_close(file->descriptor);

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

        case EVENT_DATA:
            status = ondata(&imessage.header, &omessage.header);

            break;

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

