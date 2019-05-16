#include <fudge.h>
#include <abi.h>

static void ondata(struct event_channel *channel)
{

    struct job jobs[32];

    job_interpret(jobs, 32, channel, event_getdata(channel), event_getdatasize(channel), 0);

}

void main(void)
{

    struct event_channel channel;
    unsigned int id;

    event_initsignals(&channel);
    event_setsignal(&channel, EVENT_DATA, ondata);

    if (!file_walk2(FILE_CP, "/bin/slang"))
        return;

    if (!file_walk2(FILE_C0, "/config/base.slang"))
        return;

    if (!file_walk2(FILE_C1, "/config/arch.slang"))
        return;

    if (!file_walk2(FILE_C2, "/config/init.slang"))
        return;

    id = call_spawn();

    if (!id)
        return;

    event_create(&channel.o, EVENT_INIT);
    event_place(id, &channel.o);
    event_create(&channel.o, EVENT_FILE);
    event_addfile(&channel.o, FILE_P0);
    event_place(id, &channel.o);
    event_create(&channel.o, EVENT_FILE);
    event_addfile(&channel.o, FILE_P1);
    event_place(id, &channel.o);
    event_create(&channel.o, EVENT_FILE);
    event_addfile(&channel.o, FILE_P2);
    event_place(id, &channel.o);
    event_create(&channel.o, EVENT_STOP);
    event_place(id, &channel.o);
    event_listen(&channel);

}

