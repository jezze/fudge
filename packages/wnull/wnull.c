#include <abi.h>
#include <fudge.h>

static unsigned int event_next(void *buffer)
{

    struct event_header *header = buffer;

    return header->count + sizeof (struct event_header);

}

static void sendevent(unsigned int destination, unsigned int type)
{

    struct event_header header;

    header.destination = destination;
    header.type = type;
    header.count = 0;

    call_walk(CALL_L2, CALL_PR, 17, "system/event/send");
    call_open(CALL_L2);
    call_write(CALL_L2, 0, sizeof (struct event_header), &header);
    call_close(CALL_L2);

}

static void pollevent()
{

    unsigned char buffer[FUDGE_BSIZE];
    unsigned int count, roff;
    unsigned int running = 1;

    call_walk(CALL_L1, CALL_PR, 15, "system/event/wm");
    call_open(CALL_L1);

    for (roff = 0; running && (count = call_read(CALL_L1, roff, FUDGE_BSIZE, buffer)); roff += count)
    {

        unsigned int i;

        for (i = 0; i < count; i += event_next(buffer + i))
        {

            unsigned char *temp = buffer + i;
            struct event_header *header = (struct event_header *)temp;

            switch (header->type)
            {

            case 1001:
                running = 0;

                break;

            }

        }

    }

    call_close(CALL_L1);

}

void main()
{

    sendevent(0xFFFFFFFF, 1000);
    pollevent();

    call_open(CALL_PO);
    call_write(CALL_PO, 0, 10, "Good bye!\n");
    call_close(CALL_PO);

}

