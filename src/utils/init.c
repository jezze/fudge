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
    unsigned int id;

    channel_init(&channel);
    channel_setsignal(&channel, EVENT_DATA, ondata);

    if (!file_walk2(FILE_CP, "/bin/slang"))
        return;

    id = call_spawn(FILE_CP);

    if (!id)
        return;

    channel_forward(&channel, EVENT_FILE);
    channel_append(&channel, 19, "/config/base.slang");
    channel_place(&channel, id);
    channel_forward(&channel, EVENT_FILE);
    channel_append(&channel, 19, "/config/arch.slang");
    channel_place(&channel, id);
    channel_forward(&channel, EVENT_FILE);
    channel_append(&channel, 19, "/config/init.slang");
    channel_place(&channel, id);
    channel_forward(&channel, EVENT_DONE);
    channel_place(&channel, id);
    channel_listen(&channel);

}

