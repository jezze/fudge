#include <fudge.h>
#include <abi.h>

static void ondata(struct channel *channel, unsigned int source, void *mdata, unsigned int msize)
{

    struct job jobs[32];
    unsigned int njobs = 0;
    struct job_status status;

    status.start = mdata;
    status.end = status.start + msize;

    while (status.start < status.end)
        njobs = job_parse(&status, jobs, 32);

    job_run(channel, jobs, njobs);

}

void init(struct channel *channel)
{

    unsigned int id = file_spawn("/bin/slang");

    if (id)
    {

        char *baseslang = "/config/base.slang";
        char *archslang = "/config/arch.slang";
        char *initslang = "/config/init.slang";

        channel_setcallback(channel, EVENT_DATA, ondata);
        channel_place(channel, id, EVENT_FILE, ascii_lengthz(baseslang), baseslang);
        channel_place(channel, id, EVENT_FILE, ascii_lengthz(archslang), archslang);
        channel_place(channel, id, EVENT_FILE, ascii_lengthz(initslang), initslang);
        channel_place(channel, id, EVENT_MAIN, 0, 0);

    }

}

