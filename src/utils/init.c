#include <fudge.h>
#include <abi.h>

static void ondata(struct channel *channel, unsigned int source, void *mdata, unsigned int msize)
{

    struct job jobs[32];
    unsigned int n = job_parse(jobs, 32, mdata, msize);
    unsigned int njobs = job_spawn(channel, jobs, n);

    job_pipe(channel, jobs, njobs);
    job_run(channel, jobs, njobs);

}

void init(struct channel *channel)
{

    unsigned int id = file_spawn("/bin/slang");

    if (id)
    {

        struct message_data data;

        channel_setcallback(channel, EVENT_DATA, ondata);
        channel_sendredirectback(channel, id, EVENT_DATA);
        channel_send(channel, id, EVENT_FILE, message_putstringz(&data, "/config/base.slang", 0), &data);
        channel_send(channel, id, EVENT_FILE, message_putstringz(&data, "/config/arch.slang", 0), &data);
        channel_send(channel, id, EVENT_FILE, message_putstringz(&data, "/config/init.slang", 0), &data);
        channel_send(channel, id, EVENT_MAIN, 0, 0);

    }

}

