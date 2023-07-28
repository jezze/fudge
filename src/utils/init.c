#include <fudge.h>
#include <abi.h>

#define JOBSIZE                         32

static void ondata(unsigned int source, void *mdata, unsigned int msize)
{

    struct job_worker workers[JOBSIZE];
    struct job job;

    job_init(&job, workers, JOBSIZE);
    job_parse(&job, mdata, msize);

    if (job_spawn(&job, FILE_L0, FILE_PW))
    {

        job_listen(&job, EVENT_CLOSE);
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

void init(void)
{

    unsigned int channel = call_spawn_absolute(FILE_L0, FILE_PW, "initrd:/bin/slang");

    if (channel)
    {

        channel_bind(EVENT_DATA, ondata);
        channel_listen(channel, EVENT_CLOSE);
        channel_listen(channel, EVENT_DATA);
        channel_listen(channel, EVENT_ERROR);
        channel_send_fmt0(channel, EVENT_PATH, "/config/base.slang\\0");
        channel_send_fmt0(channel, EVENT_PATH, "/config/arch.slang\\0");
        channel_send_fmt0(channel, EVENT_PATH, "/config/init.slang\\0");
        channel_send(channel, EVENT_MAIN);

    }

}

