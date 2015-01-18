#include <fudge.h>

struct event
{

    unsigned int type;
    unsigned int count;

};

void sendevent()
{

    struct event event;

    event.type = 4;
    event.count = 0;

    call_walk(CALL_L1, CALL_DR, 14, "system/event/0");
    call_open(CALL_L1);
    call_write(CALL_L1, 0, 8, &event);
    call_close(CALL_L1);

}

void main()
{

    sendevent();

}

