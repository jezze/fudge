#include <fudge.h>
#include <abi.h>

static void dump(union event_message *imessage, union event_message *omessage, unsigned int count, void *buffer)
{

    char *data = buffer;
    unsigned int i;

    for (i = 0; i < count; i++)
    {

        unsigned char num[FUDGE_NSIZE];

        event_reply(omessage, imessage, EVENT_DATA);
        event_append(omessage, ascii_wzerovalue(num, FUDGE_NSIZE, data[i], 16, 2, 0), num);
        event_place(omessage->header.target, omessage);

    }

}

static unsigned int ondata(union event_message *imessage, union event_message *omessage)
{

    dump(imessage, omessage, event_getdatasize(imessage), event_getdata(imessage));

    return 0;

}

static unsigned int oninit(union event_message *imessage, union event_message *omessage)
{

    event_request(omessage, imessage, EVENT_BLOCKREQUEST, 0);
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

    file_open(FILE_G0);

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

    file_close(FILE_G0);

}

