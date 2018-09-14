#include <abi.h>
#include <fudge.h>
#include <event/base.h>

void main(void)
{

    unsigned int id;

    if (!file_walk(FILE_L0, "/system/event"))
        return;

    if (!file_walk(FILE_CP, "/bin/slang2"))
        return;

    if (!file_walk(FILE_CI, "/config/init2.slang"))
        return;

    id = call_spawn();

    file_open(FILE_L0);
    event_sendinit(FILE_L0, id);
    event_sendrein(FILE_L0, id, FILE_PI);
    event_sendexit(FILE_L0, id);
    file_close(FILE_L0);

}

