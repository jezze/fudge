#include <abi.h>
#include <fudge.h>

struct event_header
{

    unsigned int destination;
    unsigned int source;
    unsigned int type;
    unsigned int count;

};

void sendevent()
{

    struct event_header header;

    header.type = 1000;
    header.count = 0;

    call_walk(CALL_L0, CALL_DR, 17, "system/event/send");
    call_open(CALL_L0);
    call_write(CALL_L0, 0, sizeof (struct event_header), &header);
    call_close(CALL_L0);

}

void main()
{

    sendevent();

}

