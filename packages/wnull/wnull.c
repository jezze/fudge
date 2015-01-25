#include <abi.h>
#include <fudge.h>

struct event_header
{

    unsigned int destination;
    unsigned int source;
    unsigned int type;
    unsigned int count;

};

void sendevent(unsigned int type)
{

    struct event_header header;

    header.type = type;
    header.count = 0;

    call_walk(CALL_L1, CALL_DR, 17, "system/event/send");
    call_open(CALL_L1);
    call_write(CALL_L1, 0, sizeof (struct event_header), &header);
    call_close(CALL_L1);

}

void main()
{

    sendevent(1000);

}

