#include <fudge.h>
#include <abi.h>

static struct crc s;

static unsigned int ondata(union event_message *imessage, union event_message *omessage)
{

    crc_read(&s, event_getdata(imessage), event_getdatasize(imessage));

    return 0;

}

static unsigned int onfile(union event_message *imessage, union event_message *omessage)
{

    struct event_file *file = event_getdata(imessage);
    unsigned char buffer[FUDGE_BSIZE];
    unsigned int count;

    file_open(file->descriptor);

    while ((count = file_read(file->descriptor, buffer, FUDGE_BSIZE)))
        crc_read(&s, buffer, count);

    file_close(file->descriptor);

    return 0;

}

static unsigned int onstop(union event_message *imessage, union event_message *omessage)
{

    unsigned char buffer[FUDGE_BSIZE];
    unsigned int result = crc_finalize(&s);

    event_reply(omessage, imessage, EVENT_DATA);
    event_append(omessage, ascii_wvalue(buffer, 32, result, 10), buffer);
    event_append(omessage, 1, "\n");
    event_place(omessage->header.target, omessage);

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

        case EVENT_DATA:
            status = ondata(&imessage, &omessage);

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

