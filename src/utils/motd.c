#include <fudge.h>
#include <abi.h>

static unsigned int onstop(union event_message *imessage, union event_message *omessage)
{

    unsigned int id;

    if (!file_walk2(FILE_CP, "/bin/echo"))
        return 0;

    if (!file_walk2(FILE_C0, "/data/motd.txt"))
        return 0;

    id = call_spawn();

    if (!id)
        return 0;

    event_forward(omessage, imessage, EVENT_INIT, id);
    event_send2(omessage);
    event_forward(omessage, imessage, EVENT_FILE, id);
    event_addfile(omessage, FILE_P0);
    event_send2(omessage);
    event_forward(omessage, imessage, EVENT_STOP, id);
    event_send2(omessage);

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

        event_read(&imessage);

        switch (imessage.header.type)
        {

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

