#include <abi.h>
#include <fudge.h>

static void sendevent(unsigned int destination, unsigned int type)
{

    struct event_header header;

    header.destination = destination;
    header.type = type;
    header.count = 0;

    call_walk(CALL_L2, CALL_PR, 17, "system/event/send");
    call_open(CALL_L2);
    call_write(CALL_L2, 0, sizeof (struct event_header), 1, &header);
    call_close(CALL_L2);

}

static void pollevent()
{

    struct event_header header;
    unsigned int count, roff, quit = 0;

    call_walk(CALL_L1, CALL_PR, 17, "system/event/poll");
    call_open(CALL_L1);

    for (roff = 0; (count = call_read(CALL_L1, roff, sizeof (struct event_header), 1, &header)); roff += count)
    {

        char data[32];

        if (header.count)
            count += call_read(CALL_L1, roff + count, header.count, 1, data);

        switch (header.type)
        {

        case 1001:
            quit = 1;

            break;

        }

        if (quit)
            break;

    }

    call_close(CALL_L1);

}

void main()
{

    sendevent(0xFFFFFFFF, 1000);
    pollevent();

    call_open(CALL_PO);
    call_write(CALL_PO, 0, 10, 1, "Good bye!\n");
    call_close(CALL_PO);

}

