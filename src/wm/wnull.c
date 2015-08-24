#include <abi.h>
#include <fudge.h>
#include "box.h"
#include "send.h"

static void pollevent(void)
{

    union event event;
    unsigned int count, quit = 0;

    call_walk(CALL_L1, CALL_PR, 17, "system/event/poll");
    call_open(CALL_L1);

    while ((count = call_read(CALL_L1, sizeof (struct event_header), &event.header)))
    {

        if (event.header.count)
            call_read(CALL_L1, event.header.count, event.data + sizeof (struct event_header));

        switch (event.header.type)
        {

        case EVENT_WMREADY:
            send_wmadd(CALL_L2, event.header.source, 12, "Hello World!");

            break;

        case EVENT_WMUNMAP:
            quit = 1;

            break;

        }

        if (quit)
            break;

    }

    call_close(CALL_L1);

}

void main(void)
{

    send_wmmap(CALL_L2, 0xFFFFFFFF);
    pollevent();

}

