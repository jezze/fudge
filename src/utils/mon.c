#include <fudge.h>
#include <abi.h>

static void dump(union event_message *imessage, union event_message *omessage, unsigned int count, void *buffer)
{

    char *data = buffer;
    unsigned int i;

    file_write(FILE_G1, "block:\n", 7);

    for (i = 0; i < count; i++)
    {

        char num[FUDGE_NSIZE];

        file_write(FILE_G1, num, ascii_wzerovalue(num, FUDGE_NSIZE, data[i], 16, 2, 0));

    }

    file_write(FILE_G1, "\n", 1);

}

static unsigned int ondata(union event_message *imessage, union event_message *omessage)
{

    dump(imessage, omessage, imessage->header.plength, event_getdata(imessage));

    return 0;

}

static unsigned int oninit(union event_message *imessage, union event_message *omessage)
{

    unsigned char dummy;

    event_reply(omessage, imessage, EVENT_DATA);
    event_append(omessage, 8, "testing\n");
    event_send(omessage);
    file_write(FILE_G0, &dummy, 1024);

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

    if (!file_walk2(FILE_G0, "/system/block/if:0/data"))
        return;

    if (!file_walk2(FILE_G1, "/system/console/if:0/odata"))
        return;

    file_open(FILE_G0);
    file_open(FILE_G1);
    event_open();

    while (!status)
    {

        event_read(&imessage);

        switch (imessage.header.type)
        {

        case EVENT_DATA:
            status = ondata(&imessage, &omessage);

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
    file_close(FILE_G1);
    file_close(FILE_G0);

}

