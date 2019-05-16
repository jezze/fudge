#include <fudge.h>
#include <abi.h>

static void loadscript(void)
{

    struct event_channel channel;
    unsigned int id;

    file_walk2(FILE_CP, "/bin/slang");
    file_walk2(FILE_C0, "/config/base.slang");
    file_walk2(FILE_C1, "/config/arch.slang");
    file_walk2(FILE_C2, "/config/init.slang");

    id = call_spawn();

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

}

static unsigned int ondata(struct event_channel *channel)
{

    struct job jobs[32];

    job_interpret(jobs, 32, channel, event_getdata(channel), event_getdatasize(channel), 0);

    return 0;

}

void init(void)
{

    event_initsignals();
    event_setsignal(EVENT_DATA, ondata);

}

void main(void)
{

    struct event_channel channel;

    loadscript();

    while (event_listen(&channel));

}

