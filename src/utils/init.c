#include <fudge.h>
#include <abi.h>

#define JOBSIZE                         32

static void ondata(unsigned int source, void *mdata, unsigned int msize)
{

    struct job_worker workers[JOBSIZE];
    struct job job;

    job_init(&job, workers, JOBSIZE);
    job_parse(&job, mdata, msize);

    if (job_spawn(&job))
    {

        job_listen(&job, EVENT_TERMRESPONSE);
        job_listen(&job, EVENT_ERROR);
        job_listen(&job, EVENT_DATA);
        job_pipe(&job, EVENT_DATA);
        job_run(&job);

    }

    else
    {

        job_killall(&job);

    }

}

static void onmain(unsigned int source, void *mdata, unsigned int msize)
{

    unsigned int id = fsp_walk(666, 0, "bin/slang");

    if (id)
    {

        unsigned int channel = call_spawn(666, id);

        if (channel)
        {

            channel_bind(EVENT_DATA, ondata);
            channel_listen(channel, EVENT_TERMRESPONSE);
            channel_listen(channel, EVENT_DATA);
            channel_listen(channel, EVENT_ERROR);
            channel_send_fmt0(channel, EVENT_PATH, "/config/base.slang\\0");
            channel_send_fmt0(channel, EVENT_PATH, "/config/arch.slang\\0");
            channel_send_fmt0(channel, EVENT_PATH, "/config/init.slang\\0");
            channel_send(channel, EVENT_MAIN);

        }

    }

}

void init(void)
{

    channel_autoclose(EVENT_MAIN, 0);
    channel_bind(EVENT_MAIN, onmain);

}

