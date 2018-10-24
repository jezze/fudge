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

static void loadscript(void)
{

    unsigned int id;
    char obuffer[FUDGE_BSIZE];
    struct event_header *oheader = (struct event_header *)obuffer;

    file_walk(FILE_CP, "/bin/slang");
    file_walk(FILE_C0, "/config/base.slang");

    id = call_spawn();

    event_createinit(oheader, id);
    event_send(oheader);
    event_createdatafile(oheader, id, 0, FILE_P0);
    event_send(oheader);
    event_createdatastop(oheader, id, 0);
    event_send(oheader);

    file_walk(FILE_CP, "/bin/slang");
    file_walk(FILE_C0, "/config/arch.slang");

    id = call_spawn();

    event_createinit(oheader, id);
    event_send(oheader);
    event_createdatafile(oheader, id, 0, FILE_P0);
    event_send(oheader);
    event_createdatastop(oheader, id, 0);
    event_send(oheader);

    file_walk(FILE_CP, "/bin/slang");
    file_walk(FILE_C0, "/config/init.slang");

    id = call_spawn();

    event_createinit(oheader, id);
    event_send(oheader);
    event_createdatafile(oheader, id, 0, FILE_P0);
    event_send(oheader);
    event_createdatastop(oheader, id, 0);
    event_send(oheader);

}

static unsigned int ondatapipe(struct event_header *iheader, struct event_header *oheader)
{

    struct event_datapipe *datapipe = event_getdata(iheader);

    job_run(iheader, oheader, datapipe + 1, datapipe->count);

    return 0;

}

void main(void)
{

    unsigned int status = 0;

    loadsystem();
    loadevent();
    event_open();
    loadscript();

    while (!status)
    {

        char ibuffer[FUDGE_BSIZE];
        char obuffer[FUDGE_BSIZE];
        struct event_header *iheader = event_read(ibuffer);
        struct event_header *oheader = (struct event_header *)obuffer;

        switch (iheader->type)
        {

        case EVENT_DATAPIPE:
            status = ondatapipe(iheader, oheader);

            break;

        }

    }

    event_close();

}

