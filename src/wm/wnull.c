#include <abi.h>
#include <fudge.h>
#include "box.h"
#include "send.h"

static unsigned int root;

static void pollevent(void)
{

    union event event;
    unsigned int count, roff, quit = 0;

    call_walk(CALL_L1, CALL_PR, 17, "system/event/poll");
    call_open(CALL_L1);

    for (roff = 0; (count = call_read(CALL_L1, roff, sizeof (struct event_header), 1, &event.header)); roff += count)
    {

        if (event.header.count)
            count += call_read(CALL_L1, roff + count, event.header.count, 1, event.data + sizeof (struct event_header));

        switch (event.header.type)
        {

        case EVENT_MOUSEPRESS:
            send_wmadd(root, 22, "text test Hello World!");

            break;

        case EVENT_WMREADY:
            root = event.header.source;

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

    send_wmmap(0xFFFFFFFF);
    pollevent();

}

