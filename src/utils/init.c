#include <fudge.h>
#include <abi.h>

static void ondata(struct channel *channel, void *mdata, unsigned int msize)
{

    struct job_status status;
    struct job jobs[32];

    status.start = mdata;
    status.end = status.start + msize;

    while (status.start < status.end)
    {

        unsigned int njobs = job_parse(&status, jobs, 32);

        job_run(channel, jobs, njobs);

    }

}

static void onredirect(struct channel *channel, void *mdata, unsigned int msize)
{

    struct event_redirect *redirect = mdata;

    channel_setredirect(channel, redirect->type, redirect->id);

}

void main(void)
{

    struct channel channel;
    unsigned int id;

    channel_init(&channel);
    channel_setsignal(&channel, EVENT_DATA, ondata);
    channel_setsignal(&channel, EVENT_REDIRECT, onredirect);

    if (!file_walk2(FILE_CP, "/bin/slang"))
        return;

    id = call_spawn(FILE_CP);

    if (!id)
        return;

    channel_request(&channel, EVENT_FILE);
    channel_append(&channel, 19, "/config/base.slang");
    channel_place(&channel, id);
    channel_request(&channel, EVENT_FILE);
    channel_append(&channel, 19, "/config/arch.slang");
    channel_place(&channel, id);
    channel_request(&channel, EVENT_FILE);
    channel_append(&channel, 19, "/config/init.slang");
    channel_place(&channel, id);
    channel_request(&channel, EVENT_DONE);
    channel_place(&channel, id);
    channel_listen(&channel);

}

