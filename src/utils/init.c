#include <abi.h>
#include <fudge.h>

static void loadsystem(void)
{

    file_walk2(FILE_CP, "/bin/elfload");
    file_walk2(FILE_C0, "/mod/system.ko");
    call_spawn();
    file_walk2(FILE_C0, "/mod/event.ko");
    call_spawn();
    file_walk2(FILE_L0, "/system");
    call_auth(FILE_L1, 2000, 2000);
    call_mount(FILE_L0, FILE_L1);
    file_walk2(FILE_PM, "/system/event");
    file_walk2(FILE_CM, "/system/event");

}

static void loadscript(void)
{

    unsigned int id;
    char obuffer[FUDGE_BSIZE];
    struct event_header *oheader = (struct event_header *)obuffer;

    file_walk2(FILE_CP, "/bin/slang");
    file_walk2(FILE_C0, "/config/base.slang");
    file_walk2(FILE_C1, "/config/arch.slang");
    file_walk2(FILE_C2, "/config/init.slang");

    id = call_spawn();

    event_createinit(oheader, id, 0);
    event_send(oheader);
    event_createfile(oheader, id, 0, FILE_P0);
    event_send(oheader);
    event_createfile(oheader, id, 0, FILE_P1);
    event_send(oheader);
    event_createfile(oheader, id, 0, FILE_P2);
    event_send(oheader);
    event_createstop(oheader, id, 0);
    event_send(oheader);

}

static unsigned int ondata(struct event_header *iheader, struct event_header *oheader)
{

    struct event_data *data = event_getdata(iheader);

    job_run(iheader, oheader, data + 1, data->count, 0);

    return 0;

}

void main(void)
{

    unsigned int status = 0;

    loadsystem();
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

        case EVENT_DATA:
            status = ondata(iheader, oheader);

            break;

        }

    }

    event_close();

}

