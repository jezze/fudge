#include <fudge.h>
#include <abi.h>

static void complete(struct event_header *iheader, struct event_header *oheader, unsigned int descriptor, void *name, unsigned int length)
{

    struct record record;

    file_open(descriptor);
    event_replydata(oheader, iheader);

    while (file_readall(descriptor, &record, sizeof (struct record)))
    {

        if (record.length >= length && memory_match(record.name, name, length))
        {

            if (event_avail(oheader) < record.length + 1)
            {

                event_send(oheader);
                event_replydata(oheader, iheader);

            }

            event_appenddata(oheader, record.length, record.name);
            event_appenddata(oheader, 1, "\n");

        }

        if (!file_step(descriptor))
            break;

    }

    event_send(oheader);
    file_close(descriptor);

}

static unsigned int ondata(struct event_header *iheader, struct event_header *oheader)
{

    struct event_data *data = event_getdata(iheader);

    complete(iheader, oheader, FILE_PW, data + 1, data->count);

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

