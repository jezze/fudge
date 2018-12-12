#include <fudge.h>
#include <abi.h>

static void list(union event_message *imessage, union event_message *omessage, unsigned int descriptor)
{

    struct record record;

    file_open(descriptor);
    event_reply(omessage, imessage, EVENT_DATA);

    while (file_readall(descriptor, &record, sizeof (struct record)))
    {

        char num[FUDGE_NSIZE];

        if (event_avail(omessage) < record.length + 3 + 16)
        {

            event_place(omessage->header.target, omessage);
            event_reset(omessage);

        }

        event_append(omessage, ascii_wzerovalue(num, FUDGE_NSIZE, record.id, 16, 8, 0), num);
        event_append(omessage, 1, " ");
        event_append(omessage, ascii_wzerovalue(num, FUDGE_NSIZE, record.size, 16, 8, 0), num);
        event_append(omessage, 1, " ");
        event_append(omessage, record.length, record.name);
        event_append(omessage, 1, "\n");

        if (!file_step(descriptor))
            break;

    }

    event_place(omessage->header.target, omessage);
    file_close(descriptor);

}

static unsigned int onempty(union event_message *imessage, union event_message *omessage)
{

    list(imessage, omessage, FILE_PW);

    return 0;

}

static unsigned int onfile(union event_message *imessage, union event_message *omessage)
{

    struct event_file *file = event_getdata(imessage);

    list(imessage, omessage, file->descriptor);

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

    while (!status)
    {

        switch (event_pick(&imessage))
        {

        case EVENT_EMPTY:
            status = onempty(&imessage, &omessage);

            break;

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

}

