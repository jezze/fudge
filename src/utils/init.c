#include <fudge.h>
#include <abi.h>

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

    union event_message message;
    unsigned int id;

    file_walk2(FILE_CP, "/bin/slang");
    file_walk2(FILE_C0, "/config/base.slang");
    file_walk2(FILE_C1, "/config/arch.slang");
    file_walk2(FILE_C2, "/config/init.slang");

    id = call_spawn();

    event_create(&message, EVENT_INIT, id, 0);
    event_send2(&message);
    event_create(&message, EVENT_FILE, id, 0);
    event_addfile(&message, FILE_P0);
    event_send2(&message);
    event_create(&message, EVENT_FILE, id, 0);
    event_addfile(&message, FILE_P1);
    event_send2(&message);
    event_create(&message, EVENT_FILE, id, 0);
    event_addfile(&message, FILE_P2);
    event_send2(&message);
    event_create(&message, EVENT_STOP, id, 0);
    event_send2(&message);

}

static unsigned int ondata(union event_message *imessage, union event_message *omessage)
{

    struct event_data *data = event_getdata(imessage);
    struct job jobs[32];

    job_interpret(jobs, 32, imessage, omessage, data + 1, data->count, 0);

    return 0;

}

void main(void)
{

    unsigned int status = 0;
    union event_message imessage;
    union event_message omessage;

    loadsystem();
    event_open();
    loadscript();

    while (!status)
    {

        event_read(&imessage);

        switch (imessage.header.type)
        {

        case EVENT_DATA:
            status = ondata(&imessage, &omessage);

            break;

        }

    }

    event_close();

}

