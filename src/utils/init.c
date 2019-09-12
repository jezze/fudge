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
    struct event_file file;
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


    file.descriptor = FILE_P0;

    event_create(&channel.o.header, EVENT_FILE, 0);
    channel_append(&channel, sizeof (struct event_file), &file);
    channel_place(&channel, id);

    file.descriptor = FILE_P1;

    event_create(&channel.o.header, EVENT_FILE, 0);
    channel_append(&channel, sizeof (struct event_file), &file);
    channel_place(&channel, id);

    file.descriptor = FILE_P2;

    event_create(&channel.o.header, EVENT_FILE, 0);
    channel_append(&channel, sizeof (struct event_file), &file);
    channel_place(&channel, id);
    event_create(&channel.o.header, EVENT_STOP, 0);
    channel_place(&channel, id);
    channel_listen(&channel);

}

