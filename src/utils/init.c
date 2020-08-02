#include <fudge.h>
#include <abi.h>

static void ondata(struct channel *channel, unsigned int source, void *mdata, unsigned int msize)
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

static void onredirect(struct channel *channel, unsigned int source, void *mdata, unsigned int msize)
{

    struct event_redirect *redirect = mdata;

    channel_setredirect(channel, redirect->type, redirect->mode, redirect->id, source);

}

static void oninit(struct channel *channel)
{

    unsigned int id;

    if (!file_walk2(FILE_CP, "/bin/slang"))
        return;

    id = call_spawn(FILE_CP);

    if (!id)
        return;

    channel_request(channel, EVENT_FILE);
    channel_appendstring2(channel, "/config/base.slang");
    channel_place(channel, id);
    channel_request(channel, EVENT_FILE);
    channel_appendstring2(channel, "/config/arch.slang");
    channel_place(channel, id);
    channel_request(channel, EVENT_FILE);
    channel_appendstring2(channel, "/config/init.slang");
    channel_place(channel, id);
    channel_request(channel, EVENT_MAIN);
    channel_place(channel, id);

}

void main(void)
{

    struct channel channel;

    channel_init(&channel);
    channel_setsignal(&channel, EVENT_DATA, ondata);
    channel_setsignal(&channel, EVENT_REDIRECT, onredirect);
    channel_listen2(&channel, oninit, 0);

}

