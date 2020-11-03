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

static void oninit(struct channel *channel)
{

    unsigned int id = file_spawn("/bin/slang");

    if (id)
    {

        char *base = "/config/base.slang";
        char *arch = "/config/arch.slang";
        char *init = "/config/init.slang";

        channel_place(channel, id, EVENT_FILE, ascii_length(base) + 1, base);
        channel_place(channel, id, EVENT_FILE, ascii_length(arch) + 1, arch);
        channel_place(channel, id, EVENT_FILE, ascii_length(init) + 1, init);
        channel_place(channel, id, EVENT_MAIN, 0, 0);

    }

}

void main(void)
{

    struct channel channel;

    channel_init(&channel);
    channel_setsignal(&channel, EVENT_DATA, ondata);
    channel_listen(&channel, oninit, 0);

}

