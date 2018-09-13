#include <abi.h>
#include <fudge.h>
#include <event/base.h>

void main(void)
{

    struct event redirect;
    unsigned int id;

    if (!file_walk(FILE_L1, "/system/wm/event"))
        return;

    if (!file_walk(FILE_CP, "/bin/slang2"))
        return;

    id = call_spawn();

    event_sendinit(FILE_L1, id);
    memory_copy(redirect.data, "/config/init2.slang", 20);
    event_send(FILE_L1, &redirect, id, EVENT_REIN, 20);
    event_sendexit(FILE_L1, id);

}

