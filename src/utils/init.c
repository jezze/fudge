#include <fudge.h>
#include <abi.h>

static void ondata(struct channel *channel, void *mdata, unsigned int msize)
{

    struct job jobs[32];

    job_interpret(jobs, 32, channel, mdata, msize, 0);

}

void main(void)
{

    struct channel channel;
    unsigned int id;

    channel_init(&channel);
    channel_setsignal(&channel, EVENT_DATA, ondata);

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

    event_create(&channel.o.header, EVENT_FILE);
    event_addfile(&channel.o.header, FILE_P0);
    channel_place(id, &channel.o.header);
    event_create(&channel.o.header, EVENT_FILE);
    event_addfile(&channel.o.header, FILE_P1);
    channel_place(id, &channel.o.header);
    event_create(&channel.o.header, EVENT_FILE);
    event_addfile(&channel.o.header, FILE_P2);
    channel_place(id, &channel.o.header);
    event_create(&channel.o.header, EVENT_STOP);
    channel_place(id, &channel.o.header);
    channel_listen(&channel);

}

