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

void init(struct channel *channel)
{

    unsigned int id = file_spawn("/bin/slang");

    if (id)
    {

        char *base = "/config/base.slang";
        char *arch = "/config/arch.slang";
        char *init = "/config/init.slang";

        channel_setsignal(channel, EVENT_DATA, ondata);
        channel_place(channel, id, EVENT_FILE, ascii_lengthz(base), base);
        channel_place(channel, id, EVENT_FILE, ascii_lengthz(arch), arch);
        channel_place(channel, id, EVENT_FILE, ascii_lengthz(init), init);
        channel_place(channel, id, EVENT_MAIN, 0, 0);

    }

}

