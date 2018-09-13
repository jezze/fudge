#include <abi.h>
#include <fudge.h>
#include <event/base.h>

void main(void)
{

    struct event redirect;
    unsigned int id;

    if (!file_walk(FILE_L0, "/system/event"))
        return;

    if (!file_walk(FILE_CP, "/bin/slang2"))
        return;

    id = call_spawn();

    file_open(FILE_L0);
    event_sendinit(FILE_L0, id);
    memory_copy(redirect.data, "/config/init2.slang", 20);
    event_send(FILE_L0, &redirect, id, EVENT_REIN, 20);
    event_sendexit(FILE_L0, id);
    file_close(FILE_L0);

}

