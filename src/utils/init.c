#include <abi.h>
#include <fudge.h>
#include <event/base.h>

void main(void)
{

    unsigned int id;

    if (!file_walk(FILE_CW, "/home"))
        return;

    if (!file_walk(FILE_CP, "/bin/elfload"))
        return;

    if (!file_walk(FILE_CI, "/mod/base.ko"))
        return;

    id = call_spawn();

    if (!file_walk(FILE_CI, "/mod/system.ko"))
        return;

    id = call_spawn();

    if (!file_walk(FILE_CI, "/mod/internals.ko"))
        return;

    id = call_spawn();

    if (!file_walk(FILE_CI, "/mod/event.ko"))
        return;

    id = call_spawn();



    if (!file_walk(FILE_CP, "/bin/initfs"))
        return;

    id = call_spawn();



    if (!file_walk(FILE_L0, "/system/event"))
        return;




    if (!file_walk(FILE_CP, "/bin/slang"))
        return;

    if (!file_walk(FILE_CI, "/config/init.slang"))
        return;

    id = call_spawn();

    file_open(FILE_L0);
    event_sendinit(FILE_L0, EVENT_ADDR_SELF, id);
    event_sendfile(FILE_L0, EVENT_ADDR_SELF, id, FILE_PI);
    event_sendexit(FILE_L0, EVENT_ADDR_SELF, id);
    file_close(FILE_L0);

}

