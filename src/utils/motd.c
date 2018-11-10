#include <fudge.h>
#include <abi.h>

static unsigned int onstop(struct event_header *iheader, struct event_header *oheader)
{

    unsigned int id;

    if (!file_walk2(FILE_CP, "/bin/echo"))
        return 0;

    if (!file_walk2(FILE_C0, "/data/motd.txt"))
        return 0;

    id = call_spawn();

    if (!id)
        return 0;

    event_forwardinit(oheader, iheader, id);
    event_send(oheader);
    event_forwardfile(oheader, iheader, id, FILE_P0);
    event_send(oheader);
    event_forwardstop(oheader, iheader, id);
    event_send(oheader);

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

