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

    union message message;
    unsigned int id;

    if (!file_walk2(FILE_CP, "/bin/slang"))
        return;

    id = call_spawn(FILE_CP);

    if (!id)
        return;

    message_init(&message, EVENT_FILE);
    message_appendstring2(&message, "/config/base.slang");
    channel_place(channel, &message, id);
    message_init(&message, EVENT_FILE);
    message_appendstring2(&message, "/config/arch.slang");
    channel_place(channel, &message, id);
    message_init(&message, EVENT_FILE);
    message_appendstring2(&message, "/config/init.slang");
    channel_place(channel, &message, id);
    message_init(&message, EVENT_MAIN);
    channel_place(channel, &message, id);

}

void main(void)
{

    struct channel channel;

    channel_init(&channel);
    channel_setsignal(&channel, EVENT_DATA, ondata);
    channel_setsignal(&channel, EVENT_REDIRECT, onredirect);
    channel_listen2(&channel, oninit, 0);

}

