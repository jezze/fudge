#include <fudge.h>
#include "call.h"
#include "channel.h"
#include "file.h"
#include "job.h"

static unsigned int addjob(struct job *jobs, unsigned int njobs, unsigned int id)
{

    if (id)
    {

        jobs[njobs].id = id;

        return njobs + 1;

    }

    return njobs;

}

static unsigned int clearjobs(struct job *jobs, unsigned int njobs)
{

    memory_clear(jobs, sizeof (struct job) * njobs);

    return 0;

}

static void copyroutes(struct channel *channel, struct job *jobs, unsigned int njobs, unsigned int j)
{

    unsigned int i;

    for (i = njobs; i > j + 1; i--)
        ipc_addroute(&channel->message.header, jobs[i - 1].id);

}

static void runjob(struct channel *channel, struct job *jobs, unsigned int njobs)
{

    unsigned int i;

    for (i = 0; i < njobs; i++)
    {

        struct job *job = &jobs[i];

        if (job->ninputs || job->ndatas)
        {

            unsigned int k;

            for (k = 0; k < job->ninputs; k++)
            {

                struct event_file file;

                file.descriptor = FILE_P0 + k;

                channel_request(channel, EVENT_FILE);
                copyroutes(channel, jobs, njobs, i);
                channel_append(channel, sizeof (struct event_file), &file);
                channel_place(channel, job->id);

            }

            for (k = 0; k < job->ndatas; k++)
            {

                channel_request(channel, EVENT_DATA);
                copyroutes(channel, jobs, njobs, i);
                channel_append(channel, ascii_length(job->data[k]), job->data[k]);
                channel_place(channel, job->id);

            }

        }

        else
        {

            channel_request(channel, EVENT_EMPTY);
            copyroutes(channel, jobs, njobs, i);
            channel_place(channel, job->id);

        }

    }

    if (njobs > 0)
    {

        struct job *job = &jobs[0];

        channel_request(channel, EVENT_DONE);
        copyroutes(channel, jobs, njobs, 0);
        channel_place(channel, job->id);

    }

}

void job_interpret(struct job *jobs, unsigned int njobs, struct channel *channel, void *buffer, unsigned int count)
{

    unsigned int cjobs = clearjobs(jobs, njobs);
    char *start = buffer;
    char *end = start + count;

    if (!file_walk2(FILE_L0, "/bin"))
        return;

    while (start < end)
    {

        struct job *job = &jobs[cjobs];

        switch (start[0])
        {

        case 'I':
            if (!file_walk2(FILE_C0 + job->ninputs, start + 2))
                return;

            job->ninputs++;

            break;

        case 'O':
            if (!file_walk2(FILE_C0 + job->ninputs, start + 2))
                return;

            job->ninputs++;

            break;

        case 'D':
            job->data[job->ndatas] = start + 2;
            job->ndatas++;

            break;

        case 'P':
            if (!(file_walk(FILE_CP, FILE_L0, start + 2) || file_walk2(FILE_CP, start + 2)))
                return;

            cjobs = addjob(jobs, cjobs, call_spawn(FILE_CP));

            break;

        case 'E':
            if (!(file_walk(FILE_CP, FILE_L0, start + 2) || file_walk2(FILE_CP, start + 2)))
                return;

            cjobs = addjob(jobs, cjobs, call_spawn(FILE_CP));

            runjob(channel, jobs, cjobs);

            cjobs = clearjobs(jobs, cjobs);

            break;

        }

        start += ascii_length(start) + 1;

    }

}

unsigned int job_parse(struct job_status *status, struct job_proc *procs, unsigned int n)
{

    unsigned int nprocs = 0;

    memory_clear(procs, sizeof (struct job_proc) * n);

    while (status->start < status->end)
    {

        struct job_proc *p = &procs[nprocs];

        switch (status->start[0])
        {

        case 'I':
            p->files[p->nfiles] = status->start + 2;
            p->nfiles++;

            break;

        case 'O':
            p->files[p->nfiles] = status->start + 2;
            p->nfiles++;

            break;

        case 'D':
            p->inputs[p->ninputs] = status->start + 2;
            p->ninputs++;

            break;

        case 'P':
            p->path = status->start + 2;

            nprocs++;

            break;

        case 'E':
            p->path = status->start + 2;

            nprocs++;

            status->start += ascii_length(status->start) + 1;

            return nprocs;

        }

        status->start += ascii_length(status->start) + 1;

    }

    return nprocs;

}

static void copyroutes2(struct channel *channel, struct job_proc *procs, unsigned int njobs, unsigned int j)
{

    unsigned int i;

    for (i = njobs; i > j + 1; i--)
        ipc_addroute(&channel->message.header, procs[i - 1].id);

}

void job_run(struct channel *channel, struct job_proc *procs, unsigned int n)
{

    unsigned int i;

    if (!file_walk2(FILE_L0, "/bin"))
        return;

    for (i = 0; i < n; i++)
    {

        struct job_proc *p = &procs[i];

        if (!(file_walk(FILE_CP, FILE_L0, p->path) || file_walk2(FILE_CP, p->path)))
            continue;

        p->id = call_spawn(FILE_CP);

    }

    for (i = 0; i < n; i++)
    {

        struct job_proc *p = &procs[i];

        if (p->nfiles || p->ninputs)
        {

            unsigned int k;

            for (k = 0; k < p->nfiles; k++)
            {

                channel_request(channel, EVENT_FILE);
                copyroutes2(channel, procs, n, i);
                channel_append(channel, ascii_length(p->files[k]) + 1, p->files[k]);
                channel_place(channel, p->id);

            }

            for (k = 0; k < p->ninputs; k++)
            {

                channel_request(channel, EVENT_DATA);
                copyroutes2(channel, procs, n, i);
                channel_append(channel, ascii_length(p->inputs[k]), p->inputs[k]);
                channel_place(channel, p->id);

            }

        }

        else
        {

            channel_request(channel, EVENT_EMPTY);
            copyroutes2(channel, procs, n, i);
            channel_place(channel, p->id);

        }

    }

    if (n)
    {

        struct job_proc *p = &procs[0];

        channel_request(channel, EVENT_DONE);
        copyroutes2(channel, procs, n, 0);
        channel_place(channel, p->id);

    }

}

