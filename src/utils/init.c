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

        struct message_data data;

        channel_setcallback(channel, EVENT_DATA, ondata);
        channel_place(channel, id, EVENT_FILE, message_putstringz(&data, "/config/base.slang", 0), &data);
        channel_place(channel, id, EVENT_FILE, message_putstringz(&data, "/config/arch.slang", 0), &data);
        channel_place(channel, id, EVENT_FILE, message_putstringz(&data, "/config/init.slang", 0), &data);
        channel_place(channel, id, EVENT_MAIN, 0, 0);

    }

}

