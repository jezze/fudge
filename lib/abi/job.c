#include <fudge.h>
#include "call.h"
#include "channel.h"
#include "file.h"
#include "job.h"

unsigned int job_parse(struct job *jobs, unsigned int n, void *buffer, unsigned int count)
{

    char *start = buffer;
    char *end = start + count;
    unsigned int njobs = 0;

    buffer_clear(jobs, sizeof (struct job) * n);

    while (start < end)
    {

        struct job *p = &jobs[njobs];

        switch (start[0])
        {

        case 'F':
            p->files[p->nfiles] = start + 2;
            p->nfiles++;

            break;

        case 'R':
            p->redirects[p->nredirects] = start + 2;
            p->nredirects++;

            break;

        case 'D':
            p->inputs[p->ninputs] = start + 2;
            p->ninputs++;

            break;

        case 'P':
            p->path = start + 2;

            njobs++;

            break;

        case 'E':
            start += ascii_lengthz(start);

            return njobs;

        }

        start += ascii_lengthz(start);

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

    return call_spawn();

}

void activatenext(struct channel *channel, unsigned int index, struct job *jobs, unsigned int n)
{

    if (index < n)
    {

        struct job *job = &jobs[index];

        if (job->id)
            channel_place(channel, job->id, EVENT_MAIN, 0, 0);

    }

}

unsigned int job_spawn(struct channel *channel, struct job *jobs, unsigned int n)
{

    unsigned int i;

    for (i = 0; i < n; i++)
    {

        struct job *job = &jobs[i];

        job->id = spawn(job);

        if (!job->id)
        {

            job_term(channel, jobs, i);

            return i;

        }

    }

    return n;

}

unsigned int job_run(struct channel *channel, struct job *jobs, unsigned int n)
{

    unsigned int i;

    for (i = 0; i < n; i++)
    {

        struct job *job = &jobs[i];

        if (job->id)
        {

            job_replyback(channel, job->id, EVENT_CLOSE);

            if (i < n - 1)
                job_replyto(channel, job->id, EVENT_DATA, jobs[i + 1].id);

        }

    }

    for (i = 0; i < n; i++)
    {

        struct job *job = &jobs[i];

        if (job->id)
        {

            unsigned int j;

            for (j = 0; j < job->nfiles; j++)
                channel_place(channel, job->id, EVENT_FILE, ascii_lengthz(job->files[j]), job->files[j]);

            for (j = 0; j < job->ninputs; j++)
                channel_place(channel, job->id, EVENT_DATA, ascii_length(job->inputs[j]), job->inputs[j]);

        }

    }

    activatenext(channel, 0, jobs, n);

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

        if (job->id)
            channel_place(channel, job->id, EVENT_TERM, 0, 0);

    }

}

void job_kill(struct channel *channel, struct job *jobs, unsigned int n)
{

    unsigned int i;

    for (i = 0; i < n; i++)
    {

        struct job *job = &jobs[i];

        if (job->id)
            call_kill(job->id);

    }

}

unsigned int job_count(struct channel *channel, struct job *jobs, unsigned int n)
{

    unsigned int count = 0;
    unsigned int i;

    for (i = 0; i < n; i++)
    {

        struct job *job = &jobs[i];

        if (job->id)
            count++;

    }

    return count;

}

