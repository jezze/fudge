#include <abi.h>
#include <fudge.h>
#include <event/base.h>

void main(void)
{

    char message[FUDGE_BSIZE];
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
    file_walk(FILE_PM, "/system/event");
    file_open(FILE_PM);
    event_addheader(message, EVENT_INIT, EVENT_ADDR_SELF, id);
    event_sendbuffer(message);
    event_addheader(message, EVENT_FILE, EVENT_ADDR_SELF, id);
    event_addfile(message, FILE_PI);
    event_sendbuffer(message);
    event_addheader(message, EVENT_EXIT, EVENT_ADDR_SELF, id);
    event_sendbuffer(message);
    file_close(FILE_PM);

}

