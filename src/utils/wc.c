#include <fudge.h>
#include <abi.h>

static unsigned int bytes;
static unsigned int words;
static unsigned int lines;
static unsigned int whitespace = 1;

static void sum(unsigned int count, void *buffer)
{

    char *data = buffer;
    unsigned int i;

    for (i = 0; i < count; i++)
    {

        switch (data[i])
        {

        case '\n':
            whitespace = 1;
            lines++;

            break;

        case ' ':
            whitespace = 1;

            break;

        default:
            if (whitespace)
                words++;

            whitespace = 0;

            break;

        }

        bytes++;

    }

}

static unsigned int ondata(union event_message *imessage, union event_message *omessage)
{

    sum(event_getdatasize(imessage), event_getdata(imessage));

    return 0;

}

static unsigned int onfile(union event_message *imessage, union event_message *omessage)
{

    struct event_file *file = event_getdata(imessage);
    char buffer[FUDGE_BSIZE];
    unsigned int count;

    if (!file->descriptor)
        return 0;

    file_open(file->descriptor);

    while ((count = file_read(file->descriptor, buffer, FUDGE_BSIZE)))
        sum(count, buffer);

    file_close(file->descriptor);

    return 0;

}

static unsigned int onstop(union event_message *imessage, union event_message *omessage)
{

    char num[FUDGE_NSIZE];

    event_reply(omessage, imessage, EVENT_DATA);
    event_append(omessage, ascii_wvalue(num, FUDGE_BSIZE, lines, 10), num);
    event_append(omessage, 1, "\n");
    event_append(omessage, ascii_wvalue(num, FUDGE_BSIZE, words, 10), num);
    event_append(omessage, 1, "\n");
    event_append(omessage, ascii_wvalue(num, FUDGE_BSIZE, bytes, 10), num);
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

