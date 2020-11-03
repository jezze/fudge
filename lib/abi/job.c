#include <fudge.h>
#include "call.h"
#include "channel.h"
#include "file.h"
#include "job.h"

unsigned int job_simple(struct channel *channel, char *path, unsigned int count, void *data)
{

    unsigned int id = file_spawn(path);

    if (id)
    {

        job_replyback(channel, id, EVENT_DATA);
        job_replyback(channel, id, EVENT_CLOSE);
        channel_place(channel, id, EVENT_DATA, count, data);
        channel_place(channel, id, EVENT_MAIN, 0, 0);

    }

    return id;
}

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

void job_replyto(struct channel *channel, unsigned int target, unsigned int type, unsigned int id)
{

    struct event_redirect redirect;

    redirect.type = type;
    redirect.mode = EVENT_REDIRECT_TARGET;
    redirect.id = id;

    channel_place(channel, target, EVENT_REDIRECT, sizeof (struct event_redirect), &redirect);

}

void job_replyback(struct channel *channel, unsigned int target, unsigned int type)
{

    struct event_redirect redirect;

    redirect.type = type;
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

        if (!job->id)
            continue;

        if (i < n - 1)
            job_replyto(channel, job->id, EVENT_DATA, jobs[i + 1].id);
        else
            job_replyback(channel, job->id, EVENT_DATA);

    }

    for (i = 0; i < n; i++)
    {

        struct job *job = &jobs[i];
        unsigned int i;

        if (!job->id)
            continue;

        for (i = 0; i < job->nfiles; i++)
            channel_place(channel, job->id, EVENT_FILE, ascii_length(job->files[i]) + 1, job->files[i]);

        for (i = 0; i < job->ninputs; i++)
            channel_place(channel, job->id, EVENT_DATA, ascii_length(job->inputs[i]), job->inputs[i]);

    }

    for (i = 0; i < n; i++)
    {

        struct job *job = &jobs[i];

        if (!job->id)
            continue;

        job_replyback(channel, job->id, EVENT_CLOSE);
        channel_place(channel, job->id, EVENT_MAIN, 0, 0);

    }

}

void job_wait(struct channel *channel, struct job *jobs, unsigned int n)
{

    unsigned int nids = 0;
    unsigned int i;

    for (i = 0; i < n; i++)
    {

        struct job *job = &jobs[i];

        if (job->id)
            nids++;

    }

    if (nids)
    {

        struct message_header header;
        char data[FUDGE_MSIZE];

        while (channel_polltype(channel, EVENT_CLOSE, &header, data))
        {

            if (--nids == 0)
                break;

        }

    }

}

