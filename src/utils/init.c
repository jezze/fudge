#include <fudge.h>
#include <abi.h>

static void loadscript(void)
{

    union event_message message;
    unsigned int id;

    file_walk2(FILE_CP, "/bin/slang");
    file_walk2(FILE_C0, "/config/base.slang");
    file_walk2(FILE_C1, "/config/arch.slang");
    file_walk2(FILE_C2, "/config/init.slang");

    id = call_spawn();

    event_create(&message, EVENT_INIT);
    event_place(id, &message);
    event_create(&message, EVENT_FILE);
    event_addfile(&message, FILE_P0);
    event_place(id, &message);
    event_create(&message, EVENT_FILE);
    event_addfile(&message, FILE_P1);
    event_place(id, &message);
    event_create(&message, EVENT_FILE);
    event_addfile(&message, FILE_P2);
    event_place(id, &message);
    event_create(&message, EVENT_STOP);
    event_place(id, &message);

}

static unsigned int ondata(union event_message *imessage, union event_message *omessage)
{

    struct job jobs[32];

    job_interpret(jobs, 32, imessage, omessage, event_getdata(imessage), event_getdatasize(imessage), 0);

    return 0;

}

void main(void)
{

    unsigned int status = 0;
    union event_message imessage;
    union event_message omessage;

    loadscript();

    while (!status)
    {

        switch (event_pick(&imessage))
        {

        case EVENT_DATA:
            status = ondata(&imessage, &omessage);

            break;

        }

    }

}

