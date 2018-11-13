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

    dump(imessage, omessage, event_getdatasize(imessage), event_getdata(imessage));

    return 0;

}

static unsigned int oninit(union event_message *imessage, union event_message *omessage)
{

    event_create(omessage, EVENT_BLOCKREQUEST, EVENT_BROADCAST, 0);
    event_addblockrequest(omessage, 0, 512 * 3);
    file_writeall(FILE_G0, omessage, omessage->header.length);

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

        switch (event_pick(&imessage))
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

