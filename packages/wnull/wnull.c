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

    call_walk(CALL_L0, CALL_DR, 17, "system/event/send");
    call_open(CALL_L0);
    call_write(CALL_L0, 0, 8, &event);
    call_close(CALL_L0);

}

void main()
{

    sendevent();

}

