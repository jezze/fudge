#include <abi.h>
#include <fudge.h>
#include <event/base.h>

void main(void)
{

    unsigned int id;

    if (!file_walk(FILE_L0, "/system/event"))
        return;

    if (!file_walk(FILE_CP, "/bin/echo"))
        return;

    if (!file_walk(FILE_CI, "/data/motd.txt"))
        return;

    id = call_spawn();

    event_sendinit(FILE_L0, id);
    event_sendrein(FILE_L0, id, 0);
    event_sendexit(FILE_L0, id);

}

