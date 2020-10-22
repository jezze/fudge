#include <fudge.h>
#include "call.h"
#include "channel.h"
#include "file.h"
#include "job.h"

unsigned int job_parse(struct job_status *status, struct job *jobs, unsigned int n)
{

    unsigned int njobs = 0;

    memory_clear(jobs, sizeof (struct job) * n);

    while (status->start < status->end)
    {

        struct job *p = &jobs[njobs];

        switch (status->start[0])
        {

        case 'F':
            p->files[p->nfiles] = status->start + 2;
            p->nfiles++;

            break;

        case 'R':
            p->redirects[p->nredirects] = status->start + 2;
            p->nredirects++;

            break;

        case 'D':
            p->inputs[p->ninputs] = status->start + 2;
            p->ninputs++;

            break;

        case 'P':
            p->path = status->start + 2;

            njobs++;

            break;

        case 'E':
            status->start += ascii_length(status->start) + 1;

            return njobs;

        }

        status->start += ascii_length(status->start) + 1;

    }

    return njobs;

}

static unsigned int spawn(struct job *job)
{

    if (!file_walk2(FILE_L0, "/bin"))
        return 0;

    if (!(file_walk(FILE_CP, FILE_L0, job->path) || file_walk2(FILE_CP, job->path)))
        return 0;

    return call_spawn(FILE_CP);

}

static void redirect(struct channel *channel, struct job *job, unsigned int mode, unsigned int id)
{

    struct event_redirect redirect;

    redirect.type = EVENT_DATA;
    redirect.mode = mode;
    redirect.id = id;

    channel_place(channel, job->id, EVENT_REDIRECT, sizeof (struct event_redirect), &redirect);

}

static void senddata(struct channel *channel, struct job *job)
{

    unsigned int i;

    for (i = 0; i < job->nfiles; i++)
        channel_place(channel, job->id, EVENT_FILE, ascii_length(job->files[i]) + 1, job->files[i]);

    for (i = 0; i < job->ninputs; i++)
        channel_place(channel, job->id, EVENT_DATA, ascii_length(job->inputs[i]), job->inputs[i]);

}

static void run(struct channel *channel, struct job *job)
{

    channel_place(channel, job->id, EVENT_MAIN, 0, 0);

}

void job_run(struct channel *channel, struct job *jobs, unsigned int n)
{

    unsigned int i;

    for (i = 0; i < n; i++)
    {

        struct job *job = &jobs[i];

        job->id = spawn(job);

    }

    for (i = 0; i < n; i++)
    {

        struct job *job = &jobs[i];

        if (i < n - 1)
            redirect(channel, job, 1, jobs[i + 1].id);
        else
            redirect(channel, job, 2, 0);

    }

    for (i = 0; i < n; i++)
    {

        struct job *job = &jobs[i];

        senddata(channel, job);

    }

    for (i = 0; i < n; i++)
    {

        struct job *job = &jobs[i];

        run(channel, job);

    }

}

