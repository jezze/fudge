#include <abi.h>
#include <fudge.h>
#include <event/base.h>

void main(void)
{

    unsigned int id;

    file_walk(FILE_CW, "/home");

    /* Move to base once event is merged into kernel */
    file_walk(FILE_CP, "/bin/elfload");
    file_walk(FILE_CI, "/mod/system.ko");

    id = call_spawn();

    /* Should move event into kernel */
    file_walk(FILE_CP, "/bin/elfload");
    file_walk(FILE_CI, "/mod/event.ko");

    id = call_spawn();

    /* Move to after base and arch once event is merged into kernel */
    file_walk(FILE_CP, "/bin/initfs");

    id = call_spawn();

    /* This is what init really should do */
    file_walk(FILE_CP, "/bin/slang");
    file_walk(FILE_CI, "/config/init.slang");

    id = call_spawn();

    /* Once event system is inside kernel, there is no need to use the filesystem to send events */
    file_walk(FILE_L0, "/system/event");
    file_open(FILE_L0);
    event_sendinit(FILE_L0, EVENT_ADDR_SELF, id);
    event_sendfile(FILE_L0, EVENT_ADDR_SELF, id, FILE_PI);
    event_sendexit(FILE_L0, EVENT_ADDR_SELF, id);
    file_close(FILE_L0);

}

