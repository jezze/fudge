#include <fudge.h>
#include <abi.h>

#define JOBSIZE                         32

static void ondata(unsigned int source, void *mdata, unsigned int msize)
{

    struct job_worker workers[JOBSIZE];
    struct job job;

    job_init(&job, workers, JOBSIZE);
    job_parse(&job, mdata, msize);
    job_spawn(&job);
    job_listen(&job, EVENT_DATA);
    job_pipe(&job, EVENT_DATA);
    job_run(&job);

}

void init(void)
{

    unsigned int id = file_spawn("/bin/slang");

    if (id)
    {

        channel_bind(EVENT_DATA, ondata);
        channel_redirectback(id, EVENT_DATA);
        channel_sendstringzto(id, EVENT_PATH, "/config/base.slang");
        channel_sendstringzto(id, EVENT_PATH, "/config/arch.slang");
        channel_sendstringzto(id, EVENT_PATH, "/config/init.slang");
        channel_sendto(id, EVENT_MAIN);

    }

}

