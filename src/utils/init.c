#include <abi.h>
#include <fudge.h>

static void loadsystem(void)
{

    file_walk(FILE_CP, "/bin/elfload");
    file_walk(FILE_C0, "/mod/system.ko");
    call_spawn();
    file_walk(FILE_L0, "/system");
    call_auth(FILE_L1, 2000, 2000);
    call_mount(FILE_L0, FILE_L1);

}

static void loadevent(void)
{

    file_walk(FILE_CP, "/bin/elfload");
    file_walk(FILE_C0, "/mod/event.ko");
    call_spawn();
    file_walk(FILE_PM, "/system/event");
    file_walk(FILE_CM, "/system/event");

}

void main(void)
{

    char obuffer[FUDGE_BSIZE];
    struct event_header *oheader = (struct event_header *)obuffer;
    unsigned int id;

    loadsystem();
    loadevent();
    file_walk(FILE_CP, "/bin/slang");
    file_walk(FILE_C0, "/config/init.slang");

    id = call_spawn();

    event_open();
    event_createinit(oheader, id);
    event_send(oheader);
    event_createdatafile(oheader, id, 0, FILE_P0);
    event_send(oheader);
    event_createdatastop(oheader, id, 0);
    event_send(oheader);
    event_close();

}

