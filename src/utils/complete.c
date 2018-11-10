#include <fudge.h>
#include <abi.h>

static void complete(union event_message *imessage, union event_message *omessage, unsigned int descriptor, void *name, unsigned int length)
{

    struct record record;

    file_open(descriptor);
    event_reply(omessage, imessage, EVENT_DATA);

    while (file_readall(descriptor, &record, sizeof (struct record)))
    {

        if (record.length >= length && memory_match(record.name, name, length))
        {

            if (event_avail(omessage) < record.length + 1)
            {

                event_send(omessage);
                event_reply(omessage, imessage, EVENT_DATA);

            }

            event_append(omessage, record.length, record.name);
            event_append(omessage, 1, "\n");

        }

        if (!file_step(descriptor))
            break;

    }

    event_send(omessage);
    file_close(descriptor);

}

static unsigned int ondata(union event_message *imessage, union event_message *omessage)
{

    complete(imessage, omessage, FILE_PW, event_getdata(imessage), event_getdatasize(imessage));

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

        switch (event_pick(&imessage))
        {

        case EVENT_DATA:
            status = ondata(&imessage, &omessage);

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

