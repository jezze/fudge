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

    unsigned int id = file_spawn(FILE_L0, "/bin/slang");

    if (id)
    {

        channel_bind(EVENT_DATA, ondata);
        channel_redirectback(id, EVENT_CLOSE);
        channel_redirectback(id, EVENT_DATA);
        channel_redirectback(id, EVENT_ERROR);
        channel_sendfmt0to(id, EVENT_PATH, "/config/base.slang\\0");
        channel_sendfmt0to(id, EVENT_PATH, "/config/arch.slang\\0");
        channel_sendfmt0to(id, EVENT_PATH, "/config/init.slang\\0");
        channel_sendto(id, EVENT_MAIN);

    }

}

