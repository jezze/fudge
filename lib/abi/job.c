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
            status->start += ascii_lengthz(status->start);

            return njobs;

        }

        status->start += ascii_lengthz(status->start);

    }

    return njobs;

}

void job_replyto(struct channel *channel, unsigned int target, unsigned int event, unsigned int id)
{

    struct event_redirect redirect;

    redirect.event = event;
    redirect.mode = EVENT_REDIRECT_TARGET;
    redirect.id = (channel->callbacks[event].redirect) ? (channel->callbacks[event].redirect) : id;

    channel_place(channel, target, EVENT_REDIRECT, sizeof (struct event_redirect), &redirect);

}

void job_replyback(struct channel *channel, unsigned int target, unsigned int event)
{

    struct event_redirect redirect;

    redirect.event = event;
    redirect.mode = EVENT_REDIRECT_SOURCE;
    redirect.id = 0;

    channel_place(channel, target, EVENT_REDIRECT, sizeof (struct event_redirect), &redirect);

}

static unsigned int spawn(struct job *job)
{

    if (!file_walk2(FILE_L0, "/bin"))
        return 0;

    if (!(file_walk(FILE_CP, FILE_L0, job->path) || file_walk2(FILE_CP, job->path)))
        return 0;

    return call_spawn(FILE_CP);

}

void activatenext(struct channel *channel, unsigned int index, struct job *jobs, unsigned int n)
{

    struct job *job = &jobs[index];

    if (job->id)
        channel_place(channel, job->id, EVENT_MAIN, 0, 0);

}

unsigned int job_run(struct channel *channel, struct job *jobs, unsigned int n)
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

        if (!job->id)
            continue;

        job_replyback(channel, job->id, EVENT_CLOSE);

        if (i < n - 1)
            job_replyto(channel, job->id, EVENT_DATA, jobs[i + 1].id);

    }

    for (i = 0; i < n; i++)
    {

        struct job *job = &jobs[i];
        unsigned int j;

        if (!job->id)
            continue;

        for (j = 0; j < job->nfiles; j++)
            channel_place(channel, job->id, EVENT_FILE, ascii_lengthz(job->files[j]), job->files[j]);

        for (j = 0; j < job->ninputs; j++)
            channel_place(channel, job->id, EVENT_DATA, ascii_length(job->inputs[j]), job->inputs[j]);

    }

    if (n)
    {

        activatenext(channel, 0, jobs, n);

    }

    return job_count(channel, jobs, n);

}

unsigned int job_close(struct channel *channel, unsigned int id, struct job *jobs, unsigned int n)
{

    unsigned int i;

    for (i = 0; i < n; i++)
    {

        struct job *job = &jobs[i];

        if (job->id == id)
        {

            job->id = 0;

            activatenext(channel, i + 1, jobs, n);

            break;

        }

    }

    return job_count(channel, jobs, n);

}

void job_term(struct channel *channel, struct job *jobs, unsigned int n)
{

    unsigned int i;

    for (i = 0; i < n; i++)
    {

        struct job *job = &jobs[i];

        if (!job->id)
            continue;

        channel_place(channel, job->id, EVENT_TERM, 0, 0);

    }

}

unsigned int job_count(struct channel *channel, struct job *jobs, unsigned int n)
{

    unsigned int nids = 0;
    unsigned int i;

    for (i = 0; i < n; i++)
    {

        struct job *job = &jobs[i];

        if (job->id)
            nids++;

    }

    return nids;

}

