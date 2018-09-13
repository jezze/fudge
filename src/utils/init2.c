#include <abi.h>
#include <fudge.h>
#include <event/base.h>

void main(void)
{

    struct event redirect;
    unsigned int id;
    char num = 0;

    memory_copy(redirect.data, &num, 1);

    if (!file_walk(FILE_L0, "/system/event"))
        return;

    if (!file_walk(FILE_CP, "/bin/slang2"))
        return;

    if (!file_walk(FILE_CI + num, "/config/init2.slang"))
        return;

    id = call_spawn();

    file_open(FILE_L0);
    event_sendinit(FILE_L0, id);
    event_send(FILE_L0, &redirect, id, EVENT_REIN, 1);
    event_sendexit(FILE_L0, id);
    file_close(FILE_L0);

}

