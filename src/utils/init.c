#include <fudge.h>
#include <abi.h>

static void ondata(struct channel *channel, void *mdata, unsigned int msize)
{

    struct job jobs[32];

    job_interpret(jobs, 32, channel, mdata, msize);

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

    channel_forward(&channel, EVENT_OPEN);
    channel_place(&channel, id);

    file.descriptor = FILE_P0;

    channel_forward(&channel, EVENT_FILE);
    channel_append(&channel, sizeof (struct event_file), &file);
    channel_place(&channel, id);

    file.descriptor = FILE_P1;

    channel_forward(&channel, EVENT_FILE);
    channel_append(&channel, sizeof (struct event_file), &file);
    channel_place(&channel, id);

    file.descriptor = FILE_P2;

    channel_forward(&channel, EVENT_FILE);
    channel_append(&channel, sizeof (struct event_file), &file);
    channel_place(&channel, id);
    channel_forward(&channel, EVENT_CLOSE);
    channel_place(&channel, id);
    channel_listen(&channel);

}

