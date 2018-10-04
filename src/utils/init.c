#include <abi.h>
#include <fudge.h>

void main(void)
{

    struct event_header *header;
    char message[FUDGE_BSIZE];
    unsigned int id;

    /* Move to base once event is merged into kernel */
    file_walk(FILE_CP, "/bin/elfload");
    file_walk(FILE_C0, "/mod/system.ko");

    id = call_spawn();

    /* Should move event into kernel */
    file_walk(FILE_CP, "/bin/elfload");
    file_walk(FILE_C0, "/mod/event.ko");

    id = call_spawn();

    /* Move to after base and arch once event is merged into kernel */
    file_walk(FILE_CP, "/bin/initfs");

    id = call_spawn();

    /* Setup default descriptors */
    file_walk(FILE_PM, "/system/event");
    file_walk(FILE_CM, "/system/event");

    /* This is what init really should do */
    file_walk(FILE_CP, "/bin/slang");
    file_walk(FILE_C0, "/config/init.slang");

    id = call_spawn();

    event_open();

    header = event_addheader(message, EVENT_INIT, id);

    event_send(message);

    header = event_addheader(message, EVENT_FILE, id);

    event_addfile(header, 0, FILE_P0);
    event_send(message);

    header = event_addheader(message, EVENT_EXIT, id);

    event_send(message);
    event_close();

}

