#include <fudge.h>
#include <abi.h>

static struct md5 s;

static unsigned int ondata(union event_message *imessage, union event_message *omessage)
{

    md5_read(&s, event_getdata(imessage), imessage->header.plength);

    return 0;

}

static unsigned int onfile(union event_message *imessage, union event_message *omessage)
{

    struct event_file *file = event_getdata(imessage);
    unsigned char buffer[FUDGE_BSIZE];
    unsigned int count;

    if (!file->descriptor)
        return 0;

    file_open(file->descriptor);

    while ((count = file_read(file->descriptor, buffer, FUDGE_BSIZE)))
        md5_read(&s, buffer, count);

    file_close(file->descriptor);

    return 0;

}

static unsigned int onstop(union event_message *imessage, union event_message *omessage)
{

    unsigned char digest[16];
    char num[FUDGE_NSIZE];
    unsigned int i;

    md5_write(&s, digest);
    event_reply(omessage, imessage, EVENT_DATA);

    for (i = 0; i < 16; i++)
        event_append(omessage, ascii_wzerovalue(num, FUDGE_NSIZE, digest[i], 16, 2, 0), num);

    event_append(omessage, 1, "\n");
    event_send(omessage);

    return 1;

}

static unsigned int oninit(union event_message *imessage, union event_message *omessage)
{

    md5_init(&s);

    return 0;

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

        event_read(&imessage);

        switch (imessage.header.type)
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

        case EVENT_INIT:
            status = oninit(&imessage, &omessage);

            break;

        case EVENT_KILL:
            status = onkill(&imessage, &omessage);

            break;

        }

    }

    event_close();

}

