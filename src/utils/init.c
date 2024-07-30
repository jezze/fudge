#include <fudge.h>
#include <abi.h>

#define JOBSIZE                         32

static void runinitrd(void)
{

    unsigned int channel = fsp_spawn(option_getstring("initrd"));

    if (channel)
        channel_send(channel, EVENT_MAIN);

}

static void runscripts(void)
{

    unsigned int channel = fsp_spawn(option_getstring("slang"));

    if (channel)
    {

        channel_send(channel, EVENT_MAIN);
        channel_send_fmt0(channel, EVENT_PATH, "initrd:config/base.slang\\0");
        channel_send_fmt0(channel, EVENT_PATH, "initrd:config/arch.slang\\0");
        channel_send_fmt0(channel, EVENT_PATH, "initrd:config/init.slang\\0");
        channel_send(channel, EVENT_END);

    }

}

static void ondata(unsigned int source, void *mdata, unsigned int msize)
{

    struct job_worker workers[JOBSIZE];
    struct job job;

    job_init(&job, workers, JOBSIZE);
    job_parse(&job, mdata, msize);

    if (job_spawn(&job, "initrd:bin"))
    {

        job_pipe(&job, EVENT_DATA);
        job_run(&job, "initrd:");

    }

    else
    {

        job_killall(&job);

    }

}

static void onmain(unsigned int source, void *mdata, unsigned int msize)
{

    runinitrd();
    runscripts();

}

void init(void)
{

    option_add("initrd", "initrd:bin/initrd");
    option_add("slang", "initrd:bin/slang");
    channel_bind(EVENT_MAIN, onmain);
    channel_bind(EVENT_DATA, ondata);

}

