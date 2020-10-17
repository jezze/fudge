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

    unsigned int id = file_spawn(FILE_CP, "/bin/slang");

    if (id)
    {

        struct message_header header;
        char *base = "/config/base.slang";
        char *arch = "/config/arch.slang";
        char *init = "/config/init.slang";

        message_initheader(&header, EVENT_FILE, ascii_length(base) + 1);
        channel_place2(channel, id, &header, base);
        message_initheader(&header, EVENT_FILE, ascii_length(arch) + 1);
        channel_place2(channel, id, &header, arch);
        message_initheader(&header, EVENT_FILE, ascii_length(init) + 1);
        channel_place2(channel, id, &header, init);
        message_initheader(&header, EVENT_MAIN, 0);
        channel_place2(channel, id, &header, 0);

    }

}

void main(void)
{

    struct channel channel;

    channel_init(&channel);
    channel_setsignal(&channel, EVENT_DATA, ondata);
    channel_setsignal(&channel, EVENT_REDIRECT, onredirect);
    channel_listen2(&channel, oninit, 0);

}

