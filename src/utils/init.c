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

static unsigned int ondata(struct event_channel *channel)
{

    struct job jobs[32];

    job_interpret(jobs, 32, channel, event_getdata(&channel->i), event_getdatasize(&channel->i), 0);

    return 0;

}

void main(void)
{

    unsigned int status = 0;
    struct event_channel channel;

    loadscript();

    while (!status)
    {

        switch (event_pick(&channel))
        {

        case EVENT_DATA:
            status = ondata(&channel);

            break;

        }

    }

}

